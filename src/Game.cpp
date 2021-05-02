/*Copyright (C) 2012-2014, 2021 Carsten Paproth

This file is part of Skat-Konferenz.

Skat-Konferenz is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Skat-Konferenz is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Skat-Konferenz.  If not, see <http://www.gnu.org/licenses/>.*/


#include "Game.h"
#include "Network.h"
#include "ui.h"
#include "Convenience.h"
#include <iostream>
#include <set>


using namespace SK;
using namespace CPLib;


static crypto_type modexp(crypto_type b, crypto_type e, const crypto_type& m) {
	if (m == 1)
		return 0;
	crypto_type r(1);
	b %= m;
	while (e > 0) {
		if ((e & 1) == 1)
			r = (r * b) % m;
		e >>= 1;
		b = (b * b) % m;
	}
	return r;
}


static string to_base94(crypto_type v, char o = 33) {
	string r;
	if (v == 0)
		return string(1, o);
	while (v > 0) {
		r.append(1, (v % 94).convert_to<char>() + o);
		v /= 94;
	}
	return r;
}


static crypto_type from_base94(const string& v, char o = 33) {
	crypto_type r = 0, b = 1;
	for (unsigned i = 0; i < v.length(); i++) {
		r += b * (v[i] - o);
		b *= 94;
	}
	return r;
}


Game::Game(UserInterface& ui, Network& nw) : ui(ui), network(nw) {
	ui.f["rule change"] = boost::bind(&Game::send_rules, this);
	ui.f["name change"] = boost::bind(&Game::send_name, this);
	ui.f["game select"] = boost::bind(&Game::select_game, this);
	ui.f["table event"] = boost::bind(&Game::table_event, this);

	ui.f["game bid"] = boost::bind(&Game::bid_game, this);
	ui.f["game fold"] = boost::bind(&Game::fold_game, this);
	ui.f["skat take"] = boost::bind(&Game::take_skat, this);
	ui.f["game announce"] = boost::bind(&Game::announce_game, this);

	ui.f["game dealout"] = boost::bind(&Game::dealout_game, this);
	ui.f["game disclose"] = boost::bind(&Game::disclose_hand, this);
	ui.f["game giveup"] = boost::bind(&Game::giveup_game, this);
	ui.f["game contrare"] = boost::bind(&Game::contrare_game, this);

	left = 0;
	right = 1;

	leftrules = rightrules = 0;
	check_rules();

	deck.resize(32);
	for (unsigned i = 0; i < deck.size(); i++)
		deck[i] = i;

	unsigned secret = 0;
	string(ui.secret->value()).copy((char*)&secret, sizeof(unsigned));
	rangen.seed(secret ^ (unsigned)time(0));
	shuffle();

	p = crypto_type("110649179406060405769669636260824550432278345760244835380580314415376085982993");
	g = 3;
	seckey = boost::uniform_int<crypto_type>(1, p - 2)(rangen);

	reset_game(myself);

	network.add_handler(boost::bind(&Game::handle_command, this, _1, _2, _3));
}


Game::~Game() {
	try {
		network.remove_handler();
	
	} catch (...) {}
}


bool Game::rule(unsigned r) {
	return (rules & leftrules & rightrules & r) > 0;
}


void Game::shuffle() {
	boost::random::uniform_01<double> dist;
	for (ptrdiff_t i = deck.size() - 1; i > 0; i--)
		swap(deck[i], deck[(ptrdiff_t)(dist(rangen) * (i + 1))]);
}


string Game::cards_string(const vector<uchar>& c) {
	string str(c.size(), 0);
	for (unsigned i = 0; i < c.size(); i++)
		str[i] = c[i] + 33;
	return str;
}


vector<uchar> Game::string_cards(const string& str) {
	vector<uchar> c(str.begin(), str.end());
	for (unsigned i = 0; i < c.size(); i++)
		c[i] = (c[i] - 33) & 31;
	return c;
}


string Game::encrypt(const string& m, const crypto_type& pubkey) {
	crypto_type k = boost::uniform_int<crypto_type>(1, p - 2)(rangen);
	return to_base94(modexp(g, k, p)) + " " + to_base94(((from_base94(m, 32) + 1) * modexp(pubkey, k, p)) % p);
}


string Game::decrypt(const string& c) {
	string c1, c2 = ss(c) >> c1 >> ws;
	return to_base94((from_base94(c2) * modexp(from_base94(c1), p - seckey - 1, p)) % p - 1, 32);
}


void Game::reset_game(unsigned d) {
	dealer = d;
	gtips = 0;
	playing = false;
	givingup = false;
	contrare = 0;
	
	hand.clear();
	skat.clear();
	trick.clear();
	tricks.clear();
	lefttricks.clear();
	righttricks.clear();
	playerhand.clear();
	lefthand.clear();
	righthand.clear();
	
	secretdeck.clear();
	secretcards.clear();
	dealtcards.clear();
	drawncards.clear();

	show_info("");
	show_gameinfo(string(dealer == right? "Vorhand": dealer == left? "Mittelhand": "Hinterhand") + (rounds.size() > 0 && rounds[0] == 0? ", Bock": ""));
	show_bid(false, -1, false);

	ui.hand->deactivate();
	ui.skat->deactivate();
	ui.announce->label("Spiel ansagen");
	ui.announce->deactivate();
	ui.announce->redraw();
	
	ui.dealout->color(FL_GRAY);
	ui.dealout->deactivate();
	ui.disclose->deactivate();
	ui.giveup->value(false);
	ui.giveup->deactivate();
	show_contrare("Kontra / Re", false, false);

	ui.table->show_cards(hand, skat);
	ui.table->show_trick(trick, 0);
	ui.table->show_disclosed(lefthand, righthand);
	ui.table->show_tricks(tricks, lefttricks, righttricks);
}


void Game::reset_round() {
	scores = leftscores = rightscores = 0;
	won = leftwon = rightwon = 0;
	lost = leftlost = rightlost = 0;
	row = 0;
	rounds.clear();
	header.clear();
	reset_game(myself);
	send_rules();
	send_name();
}


void Game::show_bid(bool show, unsigned bid, bool bidding) {
	if (show) {
		ui.bid->reset(bid, bidding);
		ui.bid->color(FL_GREEN);
		if (string(ui.bid->label()) == "Reizen")
			ui.bid->deactivate();
		else
			ui.bid->activate();
		ui.fold->label(listener == myself && rule(1)? "Ramschen": listener == myself? "Einpassen": "Passen");
		ui.fold->color(FL_RED);
		ui.fold->activate();
		ui.fold->redraw();
	} else {
		ui.bid->reset(bid, bidding);
		ui.bid->color(FL_GRAY);
		ui.bid->deactivate();
		ui.fold->label("Passen");
		ui.fold->color(FL_GRAY);
		ui.fold->deactivate();
		ui.fold->redraw();
	}
}


void Game::show_contrare(const char* label, bool value, bool active) {
	if (label != 0)
		ui.contrare->label(label);
	ui.contrare->value(value);
	if (active) {
		ui.contrare->color(FL_RED);
		ui.contrare->activate();
	} else {
		ui.contrare->color(FL_GRAY);
		ui.contrare->deactivate();
	}
	ui.contrare->redraw();
}


void Game::show_info(const string& info) {
	ui.info->copy_label(info.c_str());
	ui.info->redraw();
}


void Game::show_gameinfo(const string& info) {
	ui.gameinfo->copy_label(info.c_str());
	ui.gameinfo->redraw();
}


void Game::sort_hand() {
	map<uchar, set<uchar> > suits;
	set<uchar> jacks;
	bool trump = !ui.null->value() && !ui.nullouvert->value();

	for (unsigned i = 0; i < hand.size(); i++) {
		if (trump && (hand[i] & 7) == 3)
			jacks.insert(hand[i]);
		else if (trump && (hand[i] & 7) == 4)
			suits[hand[i] & 24].insert(1);
		else
			suits[hand[i] & 24].insert((hand[i] & 7) * 2);
	}

	size_t size = jacks.size() + suits[0].size() + suits[8].size() + suits[16].size() + suits[24].size();
	hand.assign(jacks.begin(), jacks.end());

	uchar nextsuit = suits[0].size() > 0 && suits[8].size() > 0? 0: 16;
	uchar trumpsuit = ui.diamonds->value()? 24: ui.hearts->value()? 16: ui.spades->value()? 8: ui.clubs->value()? 0: 32;
	if (trumpsuit != 32) {
		for (set<uchar>::iterator it = suits[trumpsuit].begin(); it != suits[trumpsuit].end(); it++)
			hand.push_back(trumpsuit + (*it == 1? 4: *it / 2));
		suits.erase(trumpsuit);
		nextsuit = trumpsuit < 16? 16: 0;
	}
	
	while (hand.size() < size) {
		if (suits[nextsuit].size() == 0) {
			nextsuit = (nextsuit + 8) & 24;
			continue;
		}
		for (set<uchar>::iterator it = suits[nextsuit].begin(); it != suits[nextsuit].end(); it++)
			hand.push_back(nextsuit + (*it == 1? 4: *it / 2));
		suits.erase(nextsuit);
		nextsuit = (nextsuit + 16) & 24;
	}
}


string Game::game_name(bool select) {
	if (select) {
		gname = ui.diamonds->value()? 24: ui.hearts->value()? 16: ui.spades->value()? 8: ui.clubs->value()? 0: ui.grand->value()? 32: ui.null->value()? 64: 128;
		gextra = ui.ouvert->value()? 15: ui.schwarz->value()? 7: ui.schneider->value()? 3: ui.skat->active()? 1: 0;
	}
	
	string name = gname == 24? "Karo": gname == 16? "Herz": gname == 8? "Pik": gname == 0? "Kreuz": gname == 32? "Grand": gname == 64? "Null": "Null Ouvert";
	name += gextra == 15? " Ouvert": gextra == 7? " Schwarz": gextra == 3? " Schneider": gextra == 1? " Hand": "";
	name += gtips == 0? "": (gtips < 0? " ohne ": " mit ") + string(ss(abs(gtips)));

	return name;
}


void Game::check_rules() {
	rules = (ui.foldrule->value()? 1: 0) | (ui.contrarerule->value()? 2: 0) | (ui.bockrule->value()? 4: 0) | (ui.junkrule->value()? 8: 0);
	rounds.resize(remove(rounds.begin(), rounds.end(), rule(4)? 2: 0) - rounds.begin());
	rounds.resize(remove(rounds.begin(), rounds.end(), rule(8)? 2: 1) - rounds.begin());
}


void Game::send_rules() {
	check_rules();
	network.command(left, "rules", ss(rules) << ' ' << to_base94(modexp(g, seckey, p)));
	network.command(right, "rules", ss(rules) << ' ' << to_base94(modexp(g, seckey, p)));
}


void Game::send_name() {
	network.command(left, "name", ui.name->value());
	network.command(right, "name", ui.name->value());
}


void Game::select_game() {
	if (ui.null->value() || ui.nullouvert->value())
		ui.hand->deactivate();
	else if (ui.skat->active())
		ui.hand->activate();

	if (!ui.hand->active()) {
		ui.ouvert->value(false);
		ui.schwarz->value(false);
		ui.schneider->value(false);
	}

	if (ui.announce->active() || skat.size() > 0) {
		ui.announce->copy_label((game_name(true) + " ansagen").c_str());
		ui.announce->redraw();
	}

	sort_hand();
	ui.table->show_cards(hand, skat);
}


bool Game::permit_card(uchar card) {
	if (trick.size() >= 3)
		return false;
	if (trick.size() == 0 && starter != myself)
		return false;
	if (trick.size() == 1 && starter != right)
		return false;
	if (trick.size() == 2 && starter != left)
		return false;
		
	if (trick.size() == 0)
		return true;

	bool trump = gname < 64 && ((trick[0] & 7) == 3 || (trick[0] & 24) == gname);
	unsigned suit = trick[0] & 24;
	bool canserve = false;

	for (unsigned i = 0; i < hand.size(); i++)
		if (trump)
			canserve |= (hand[i] & 7) == 3 || (hand[i] & 24) == gname;
		else
			canserve |= (hand[i] & 24) == suit && (gname > 32 || (hand[i] & 7) != 3);

	if (!canserve)
		return true;
	if (trump && ((card & 7) == 3 || (card & 24) == gname))
		return true;
	if (!trump && (card & 24) == suit && (gname > 32 || (card & 7) != 3))
		return true;

	return false;
}


void Game::game_over() {
	int score;
	string result = "";
	unsigned ptricks = player == myself? tricks.size(): player == left? lefttricks.size(): righttricks.size();
	unsigned otricks = tricks.size() + lefttricks.size() + righttricks.size() - ptricks;
	uchar values[] = {11, 4, 3, 2, 10, 0, 0, 0};
	unsigned order[] = {4, 6, 7, 0, 5, 8, 9, 10};
	unsigned sum = 0, lsum = 0, rsum = 0;
	bool bock = false;

	if (!playing) {
		score = 0;
		show_info("Spiel eingepasst.");

	} else if (gname > 32) {
		score = gname == 128 && gextra == 1? 59: gname == 128? 46: gextra == 1? 35: 23;
		if (ptricks > 0) {
			score *= -2;
			show_info(player == myself? "Verloren!": (player == left? leftname: rightname) + " hat verloren.");
		} else if (otricks == 30) {
			show_info(player == myself? "Gewonnen!": (player == left? leftname: rightname) + " hat gewonnen.");
		} else {
			return;
		}
		show_gameinfo(ss(game_name(false)) << (contrare == 4? ", Re": contrare == 2? ", Kontra": ""));
		
	} else if (gextra == 31 && otricks + ptricks == 30) {
		for (unsigned i = 0; i < tricks.size(); i++)
			sum += values[tricks[i] & 7];
		for (unsigned i = 0; i < lefttricks.size(); i++)
			lsum += values[lefttricks[i] & 7];
		for (unsigned i = 0; i < righttricks.size(); i++)
			rsum += values[righttricks[i] & 7];
		
		if (ptricks == 30 || lefttricks.size() == 30 || righttricks.size() == 30)
			score = 120;
		else if (ptricks == 0 || lefttricks.size() == 0 || righttricks.size() == 0)
			score = 46;
		else
			score = 23;
			
		if (ptricks == 30)
			show_info("Durchmarsch!");
		else if (lefttricks.size() != 30 && righttricks.size() != 30 && sum <= lsum && sum <= rsum)
			show_info(ptricks == 0? "Jungfrau!": "Gewonnen!");
		else
			show_info("Nicht gewonnen.");
		show_gameinfo(ss("Ramsch, ") << sum << " Augen");

	} else if (otricks + ptricks == 30) {
		set<uchar> cards(deck.begin(), deck.end());
		for (unsigned i = 0; i < tricks.size(); i++) {
			cards.erase(tricks[i]);
			sum += values[tricks[i] & 7];
		}
		for (unsigned i = 0; i < lefttricks.size(); i++) {
			cards.erase(lefttricks[i]);
			lsum += values[lefttricks[i] & 7];
		}
		for (unsigned i = 0; i < righttricks.size(); i++) {
			cards.erase(righttricks[i]);
			rsum += values[righttricks[i] & 7];
		}
		unsigned& psum = player == myself? sum: player == left? lsum: rsum;
		for (set<uchar>::iterator it = cards.begin(); it != cards.end(); it++) {
			playerhand.push_back(*it);
			(player == myself? tricks: player == left? lefttricks: righttricks).push_back(*it);
			psum += values[*it & 7];
		}

		set<uchar> trumps;
		for (unsigned i = 0; i < playerhand.size(); i++) {
			if ((playerhand[i] & 7) == 3)
				trumps.insert(playerhand[i] / 8);
			else if ((playerhand[i] & 24) == gname)
				trumps.insert(order[playerhand[i] & 7]);
		}

		gtips = 0;
		for (set<uchar>::iterator it = trumps.begin(); it != trumps.end() && *it == gtips; it++, gtips++);
		if (trumps.begin() == trumps.end())
			gtips = gname == 32? -4: -11;
		else if (*trumps.begin() != 0)
			gtips = -*trumps.begin();
		score = abs(gtips) + 1;
		
		if (gextra > 0)
			score++;
		if (gextra >= 3)
			score += 2;
		else if (psum >= 90 || psum <= 30)
			score++;
		if (gextra >= 7)
			score += 2;
		else if (otricks == 0 || ptricks == 0)
			score++;
		if (gextra == 15)
			score++;

		result = gextra < 7 && (otricks == 0 || ptricks == 0)? " Schwarz": gextra < 3 && (psum >= 90 || psum <= 30)? " Schneider": "";
		uchar gvalue = gname == 0? 12: gname == 8? 11: gname == 16? 10: gname == 24? 9: 24;
		if ((int)bid > score * gvalue) {
			result = " Überreizt";
			score = ((bid - 1) / gvalue + 1) * gvalue * -2;
		} else if ((gextra >= 7 && otricks > 0) || (gextra >= 3 && psum < 90) || psum <= 60) {
			score *= gvalue * -2;
		} else {
			score *= gvalue;
		}
		show_info((player == myself? (score < 0? "Verloren": "Gewonnen"): (player == left? leftname: rightname) + (score < 0? " verliert": " gewinnt")) + result + "!");
		show_gameinfo(ss(game_name(false)) << ", " << psum << " Augen" << (contrare == 4? ", Re": contrare == 2? ", Kontra": ""));
		bock = score >= 100 || psum == 60;

	} else {
		return;
	}
	bock |= (score < 0 && contrare == 2) || contrare == 4;
	ui.table->show_tricks(tricks, lefttricks, righttricks);

	for (unsigned i = 0; i < 6 && row > 0; i++)
		ui.listing->remove(ui.listing->size());

	string h = ss("\t\t@b@c") << ui.name->value() << "\t@b@c" << leftname << "\t@b@c" << rightname << "\t@b@c" << (rule(1)? "E": "") << (rule(2)? "K": "") << (rule(4)? "B": "") << (rule(8)? "R": "");
	if (h != header) {
		header = h;
		row = 0;
		ui.listing->add("@-");
		ui.listing->add(header.c_str());
	}

	if (row++ % 3 == 0)
		ui.listing->add("@-");
	
	string s = contrare == 4? "Re": contrare == 2? "Ko": ""; 
	score *= contrare > 0? contrare: 1;
	if (rounds.size() > 0) {
		score *= 2;
		s += "2x";
		rounds.erase(rounds.begin());
	}
	
	if (bock && rule(4))
		rounds.resize(rounds.size() + 3, 0);
	if (bock && rule(8))
		rounds.resize(rounds.size() + 3, 1);

	if (!playing) {
		ui.listing->add(ss("Eingepasst\t@c-\t@c-\t@c-\t@c-\t@c") << s | c_str);
	} else if (gextra == 31) {
		string m = tricks.size() == 30 || (lefttricks.size() != 30 && righttricks.size() != 30 && sum <= lsum && sum <= rsum)? won++, ss(scores += score): ss("-");
		string l = lefttricks.size() == 30 || (tricks.size() != 30 && righttricks.size() != 30 && lsum <= sum && lsum <= rsum)? leftwon++, ss(leftscores += score): ss("-");
		string r = righttricks.size() == 30 || (lefttricks.size() != 30 && tricks.size() != 30 && rsum <= sum && rsum <= lsum)? rightwon++, ss(rightscores += score): ss("-");
		ui.listing->add(ss("Ramsch\t@c") << score << "\t@c" << m << "\t@c" << l << "\t@c" << r << "\t@c" << s | c_str);
	} else {
		string m = player == myself? (score > 0? won: lost)++, ss(scores += score): ss("-");
		string l = player == left? (score > 0? leftwon: leftlost)++, ss(leftscores += score): ss("-");
		string r = player == right? (score > 0? rightwon: rightlost)++, ss(rightscores += score): ss("-");
		ui.listing->add(ss(game_name(false) + result) << "\t@c" << score << "\t@c" << m << "\t@c" << l << "\t@c" << r << "\t@c" << s | c_str);
	}

	int mw = (won - lost) * 50, lw = (leftwon - leftlost) * 50, rw = (rightwon - rightlost) * 50, ml = (leftlost + rightlost) * 40, ll = (lost + rightlost) * 40, rl = (lost + leftlost) * 40;
	ui.listing->add("@-");
	ui.listing->add(ss("Gewonnen / Verloren\t\t@c") << won << " / " << lost << "\t@c" << leftwon << " / " << leftlost << "\t@c" << rightwon << " / " << rightlost << "\t" | c_str);
	ui.listing->add(ss("(Gewonnen - Verloren) x 50\t\t@c") << mw << "\t@c" << lw << "\t@c" << rw << "\t" | c_str);
	ui.listing->add(ss("(Gegner Verloren) x 40\t\t@c") << ml << "\t@c" << ll << "\t@c" << rl << "\t" | c_str);
	ui.listing->add("@-");
	ui.listing->add(ss("@bGesamt\t\t@b@c") << scores + mw + ml << "\t@b@c" << leftscores + lw + ll << "\t@b@c" << rightscores + rw + rl << "\t" | c_str);

	ui.listing->select(ui.listing->size() - 6);
	
	playing = false;
	ui.contrare->deactivate();
	ui.giveup->deactivate();
	ui.disclose->deactivate();
	if (dealer == right) {
		ui.dealout->color(FL_GREEN);
		ui.dealout->activate();
	}
}


void Game::check_trick() {
	if (trick.size() == 0)
		starter = dealer == myself? left: dealer == left? right: myself;
	else
		ui.table->show_trick(trick, starter == left? 1: starter == right? 2: 0);

	if (tricks.size() + lefttricks.size() + righttricks.size() + trick.size() >= (player == myself? 7u: 4u))
		show_contrare(contrare == 4? "Re": contrare == 2? "Kontra": "Kontra / Re", contrare > 1, false);

	if (trick.size() == 3) {
		map<unsigned, unsigned> priority;

		for (unsigned i = 0; i < trick.size(); i++) {
			uchar suit = trick[i] & 24;
			uchar value = gname > 32? (trick[i] & 7): (trick[i] & 7) == 1? 2: (trick[i] & 7) == 2? 3: (trick[i] & 7) == 4? 1: (trick[i] & 7);

			if (gname < 64 && (trick[i] & 7) == 3)
				priority[trick[i]] = i;
			else
				priority[suit + value + (suit == gname? 32: suit == (trick[0] & 24)? 64: 96)] = i;
		}
		
		if (priority.begin()->second == 1)
			starter = starter == myself? left: starter == left? right: myself;
		else if (priority.begin()->second == 2)
			starter = starter == myself? right: starter == left? myself: left;

		for (unsigned i = 0; i < trick.size(); i++)
			(starter == myself? tricks: starter == left? lefttricks: righttricks).push_back(trick[i]);
			
		trick.clear();
		game_over();
	}

	if (playing) {
		unsigned pos[] = {myself, right, left, myself, right};
		show_info(starter == pos[trick.size()]? "Spiele eine Karte!": "Warte auf Karte von " + (starter == pos[trick.size() + 2]? leftname: rightname) + '.');
	}
}


void Game::table_event() {
	unsigned sel = ui.table->selection();
	
	if (skat.size() > 1) {
		if (sel >= 100 && sel <= 101) {
			hand.push_back(skat[sel - 100]);
			skat[sel - 100] = 32;
		} else if (sel < hand.size() && skat[0] == 32) {
			skat[0] = hand[sel];
			hand.erase(hand.begin() + sel);
		} else if (sel < hand.size() && skat[1] == 32) {
			skat[1] = hand[sel];
			hand.erase(hand.begin() + sel);
		} else if (sel < hand.size()) {
			static unsigned p = 1;
			swap (hand[sel], skat[p = p? 0: 1]);
		}

		if (skat[0] == 32 || skat[1] == 32)
			ui.announce->deactivate();
		else
			ui.announce->activate();

		sort_hand();
		ui.table->show_cards(hand, skat);
		
	} else if (playing && sel < hand.size() && permit_card(hand[sel])) {

		trick.push_back(hand[sel]);
		hand.erase(hand.begin() + sel);
		network.command(left, "trick", cards_string(trick));
		network.command(right, "trick", cards_string(trick));

		ui.table->show_cards(hand, skat);
		check_trick();
	}
}


void Game::single_player() {
	player = myself;
	show_info(ss("Spiel für ") << bid << " erhalten.");
	network.command(left, "bidvalue", ss(bid));
	network.command(right, "bidvalue", ss(bid));

	ui.skat->activate();
	ui.announce->activate();
	select_game();
}


void Game::junk_player() {
	player = myself;
	playing = true;
	gname = 32;
	gextra = 31;
	if (ui.null->value() || ui.nullouvert->value()) {
		ui.grand->setonly();
		select_game();
	}
	show_gameinfo("Ramsch");
	check_trick();
}


void Game::bid_game() {
	string type = ss(ui.bid->label()) >> bid >> ws;
	show_bid(false, bid, type == "Reizen");
	show_info("");

	if (listener == myself)
		single_player();
	else if (type == "Reizen")
		network.command(listener, "bid", ss(bid));
	else
		network.command(listener, "hold", ss(bid));
}


void Game::fold_game() {
	string type = ss(ui.bid->label()) >> bid >> ws;
	bid = bid == 0? 264: bid;
	show_bid(false, -1, false);
	show_info("");
	contrare = rule(2) && bid > 18? 1: 0;

	if (listener == myself) {
		network.command(left, "nobid", rule(1)? "junk": "");
		network.command(right, "nobid", rule(1)? "junk": "");
		if (rule(1))
			junk_player();
		else
			game_over();
	} else if (type == "Reizen")
		network.command(listener, "fold", ss(bid));
	else
		network.command(listener, "fold", ss(bid + 1));
}


void Game::take_skat() {
	if (dealer == myself || dealer == left)
		network.command(right, "dealskat", "");
	else
		network.command(left, "dealskat", "");

	ui.hand->deactivate();
	ui.skat->deactivate();
	select_game();
}


void Game::announce_game() {
	if (gname > 32 && (int)bid > (gname == 128 && gextra == 1? 59: gname == 128? 46: gextra == 1? 35: 23))
		return;

	playerhand = hand;
	skat.clear();

	ui.table->show_cards(hand, skat);
	playing = true;
	check_trick();
	
	show_gameinfo(game_name(true));
	network.command(left, "announce", ss(gname) << ' ' << gextra);
	network.command(right, "announce", ss(gname) << ' ' << gextra);
	if (gname == 128 || gextra == 15) {
		network.command(left, "disclose", cards_string(hand));
		network.command(right, "disclose", cards_string(hand));
	}

	ui.hand->deactivate();
	ui.skat->deactivate();
	ui.announce->deactivate();
	ui.giveup->activate();
	if (gname != 128 && gextra != 15)
		ui.disclose->activate();
	select_game();
}


void Game::dealout_game() {
	reset_game(myself);
	shuffle();

	secretdeck = deck;
	network.command(left, "newdeal", "");
}


void Game::disclose_hand() {
	network.command(left, "disclose", cards_string(hand));
	network.command(right, "disclose", cards_string(hand));
	ui.disclose->deactivate();
}


void Game::giveup_game() {
	if (player == myself || (givingup && ui.giveup->value())) {
		quitter = myself;
		network.command(left, "giveup", "");
		network.command(right, "giveup", "");
		if (player != myself || ui.disclose->active())
			disclose_hand();
		ui.giveup->deactivate();
		ui.contrare->deactivate();
	} else {
		network.command(player == left? right: left, "givingup", ui.giveup->value()? "1": "0");
	}
}


void Game::contrare_game() {
	contrare = player == myself? 4: 2;
	network.command(left, "contrare", "");
	network.command(right, "contrare", "");
	show_contrare(0, true, false);
}


void Game::deal_cards(unsigned n, bool cipher) {
	vector<uchar> cards(deck);
	shuffle();

	dealtcards.clear();
	for (unsigned i = 0; dealtcards.size() != n && drawncards.size() < deck.size() && i < deck.size(); i++) {
		if (find(drawncards.begin(), drawncards.end(), cards[deck[i]]) == drawncards.end()) {
			dealtcards.push_back(cipher? deck[i]: cards[deck[i]]);
			drawncards.push_back(cards[deck[i]]);
		}
	}
}


void Game::decipher_cards() {
	if (secretdeck.size() == 0 || secretcards.size() == 0)
		return;

	for (unsigned i = 0; i < secretcards.size() && secretcards[i] < secretdeck.size(); i++)
		(secretcards.size() == 2? skat: hand).push_back(secretdeck[secretcards[i]]);

	sort_hand();
	ui.table->show_cards(hand, skat);

	secretdeck.clear();
	secretcards.clear();

	if (drawncards.size() == 10) {
		shuffle();
		network.command(right, "cipherdeck", encrypt(cards_string(deck), rightkey));
		deal_cards(10, true);
		network.command(left, "secretcards", encrypt(cards_string(dealtcards) + " " + cards_string(drawncards), leftkey));

	} else if (drawncards.size() == 20) {
		deal_cards(10, false);
		network.command(left, "secretcards", encrypt(cards_string(dealtcards), leftkey));
		network.command(right, "drawncards", encrypt(cards_string(drawncards), rightkey));

	} else if (drawncards.size() == 0) {
		secretdeck = deck;
	}
}


bool Game::handle_command(unsigned i, const string& command, const string& data) {
	UILock lock;

	if (command == "peersconnected" && data == "2") {
		cout << "3 peers connected with each other, the game can start!" << endl;
		network.command(0, "seat", "left");
		network.command(1, "seat", "right");
		reset_round();
		ui.dealout->color(FL_GREEN);
		ui.dealout->activate();

	} else if (command == "rules") {
		(i == left? leftkey: rightkey) = from_base94(ss(data) >> (i == left? leftrules: rightrules) >> ws);
		check_rules();

	} else if (command == "name") {
		leftname = i == left? data: leftname;
		rightname = i == right? data: rightname;

	} else if (command == "seat") {
		left = data == "left"? 1: 0;
		right = data == "right"? 1: 0;
		reset_round();


	} else if (command == "newdeal") {
		reset_game(right);
		network.command(left, "cutdeck", "");

	} else if (command == "cutdeck") {
		reset_game(left);
		shuffle();
		network.command(left, "cipherdeck", encrypt(cards_string(deck), leftkey));
		deal_cards(10, true);
		network.command(right, "secretcards", encrypt(cards_string(dealtcards) + " " + cards_string(drawncards), rightkey));

	} else if (command == "cipherdeck") {
		vector<uchar> cards(string_cards(decrypt(data)));
		for (unsigned j = 0; j < cards.size(); j++)
			cards[j] = deck[cards[j]];
		network.command(i? 0: 1, "secretdeck", encrypt(cards_string(cards), i == left? rightkey: leftkey));

	} else if (command == "secretdeck" && i == dealer) {
		secretdeck = string_cards(decrypt(data));
		decipher_cards();

	} else if (command == "secretcards" && i != dealer) {
		string secret;
		drawncards = string_cards(ss(decrypt(data)) >> secret >> ws);
		secretcards = string_cards(secret);
		decipher_cards();

	} else if (command == "drawncards" && i != dealer) {
		drawncards = string_cards(decrypt(data));
		if (drawncards.size() == 30 && rounds.size() > 0 && rounds[0] == 1) {
			network.command(left, "nobid", "junk");
			network.command(right, "nobid", "junk");
			junk_player();
		} else if (drawncards.size() == 30) {
			show_info("Warte auf Gebot von " + leftname + '.');
			network.command(left, "bidme", ss(bid = 18));
		}

	} else if (command == "dealskat" && drawncards.size() == 30 && !playing) {
		if (i == dealer) {
			deal_cards(2, false);
			network.command(i, "secretcards", encrypt(cards_string(dealtcards), i == left? leftkey: rightkey));
			network.command(i? 0: 1, "drawncards", encrypt(cards_string(drawncards), i == left? rightkey: leftkey));
		} else {
			shuffle();
			network.command(dealer, "cipherdeck", encrypt(cards_string(deck), dealer == left? leftkey: rightkey));
			deal_cards(2, true);
			network.command(i, "secretcards", encrypt(cards_string(dealtcards) + " " + cards_string(drawncards), i == left? leftkey: rightkey));
		}


	} else if (command == "bidme") {
		ss(data) >> bid;
		listener = i;
		show_info("Reize " + (i == left? leftname: rightname) + '!');
		show_bid(true, bid, true);

	} else if (command == "bid") {
		ss(data) >> bid;
		listener = i;
		show_info(ss(i == left? leftname: rightname) << " sagt " << bid << '!');
		show_bid(true, bid, false);
		network.command(i? 0: 1, "bidinfo", data);
		
	} else if (command == "bidinfo") {
		show_info((i == left? rightname: leftname) + " sagt " + data + " zu " + (i == left? leftname: rightname) + '.');

	} else if (command == "hold") {
		ss(data) >> bid;
		listener = i;
		show_info(ss(i == left? leftname: rightname) << " hält " << bid << '.');
		show_bid(true, bid + 1, true);

	} else if (command == "fold") {
		if (i == dealer && bid == 18 && data == "18") {
			show_info(ss("Spielen für 18 oder ") << (rule(1)? "Ramschen?": "Einpassen?"));
			listener = myself;
			show_bid(true, bid, false);
		} else if (i == dealer || dealer == myself) {
			single_player();
		} else {
			show_info(ss(i == left? leftname: rightname) << " passt bei " << bid << '.');
			network.command(dealer, "bidme", data);
		}

	} else if (command == "nobid") {
		if (data == "junk")
			junk_player();
		else
			game_over();
		

	} else if (command == "bidvalue") {
		ss(data) >> bid;
		player = i;
		show_info("Warte auf Spielansage.");
		show_gameinfo(ss(i == left? leftname: rightname) << " spielt für " << bid << '.');
		
	} else if (command == "announce") {
		ss(data) >> gname >> gextra;
		(gname == 24? ui.diamonds: gname == 16? ui.hearts: gname == 8? ui.spades: gname == 0? ui.clubs: gname == 32? ui.grand: gname == 64? ui.null: ui.nullouvert)->setonly();
		select_game();
		show_gameinfo((player == left? leftname : rightname) + " spielt " + game_name(false) + '.');
		ui.giveup->activate();
		if (contrare > 0)
			show_contrare("Kontra", false, true);
		playing = true;
		check_trick();

	} else if (command == "trick") {
		trick = string_cards(data);
		if (i == player && !trick.empty() && playerhand.size() < 10)
			playerhand.push_back(trick.back());
		vector<uchar>& cards = i == left? lefthand: righthand;
		if (!trick.empty())
			cards.resize(remove(cards.begin(), cards.end(), trick.back()) - cards.begin());
		ui.table->show_disclosed(lefthand, righthand);
		check_trick();

	} else if (command == "disclose") {
		vector<uchar> cards = string_cards(data);
		for (unsigned j = 0; j < cards.size() && i == player; j++)
			playerhand.push_back(cards[j]);
		(i == left? lefthand: righthand) = cards;
		ui.table->show_disclosed(lefthand, righthand);

		if (hand.size() + lefthand.size() + righthand.size() + trick.size() + tricks.size() + lefttricks.size() + righttricks.size() == 30) {
			vector<uchar>* t;
			if ((gname < 64 && quitter == player) || (gname > 32 && quitter != player))
				t = player == myself? &lefttricks: &tricks;
			else
				t = player == myself? &tricks: player == left? &lefttricks: &righttricks;

			for (unsigned j = 0; j < hand.size(); j++)
				t->push_back(hand[j]);
			for (unsigned j = 0; j < lefthand.size(); j++)
				t->push_back(lefthand[j]);
			for (unsigned j = 0; j < righthand.size(); j++)
				t->push_back(righthand[j]);
			for (unsigned j = 0; j < trick.size(); j++)
				t->push_back(trick[j]);

			trick.clear();
			game_over();
		}

	} else if (command == "giveup" && ui.giveup->active()) {
		quitter = i;
		disclose_hand();
		ui.giveup->deactivate();
		ui.contrare->deactivate();

	} else if (command == "givingup") {
		ss(data) >> givingup;

	} else if (command == "contrare" && contrare < 4) {
		contrare = i == player? 4: 2;
		if (player == myself)
			show_contrare("Re", false, true);
		else
			show_contrare(i != player? "Kontra": "Re", true, false);


	} else {
		return false;
	}
		
	Fl::awake();
	return true;
}
