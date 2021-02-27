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
// generated by Fast Light User Interface Designer (fluid) version 1.0303

#include "ui.h"

UILock::UILock() {
  Fl::lock();
}

UILock::~UILock() {
  Fl::unlock();
}

void UserInterface::cb_mainwnd_i(Fl_Double_Window*, void*) {
  mainwnd->hide();
}
void UserInterface::cb_mainwnd(Fl_Double_Window* o, void* v) {
  ((UserInterface*)(o->user_data()))->cb_mainwnd_i(o,v);
}

void UserInterface::cb_table_i(SK::GlTable*, void*) {
  f["table event"]();
}
void UserInterface::cb_table(SK::GlTable* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->user_data()))->cb_table_i(o,v);
}

void UserInterface::cb_bid_i(SK::BidButton*, void*) {
  f["game bid"]();
}
void UserInterface::cb_bid(SK::BidButton* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_bid_i(o,v);
}

void UserInterface::cb_fold_i(Fl_Button*, void*) {
  f["game fold"]();
}
void UserInterface::cb_fold(Fl_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_fold_i(o,v);
}

void UserInterface::cb_diamonds_i(Fl_Button*, void*) {
  f["game select"]();
}
void UserInterface::cb_diamonds(Fl_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_diamonds_i(o,v);
}

#include <FL/Fl_Pixmap.H>
static const char *idata_diamonds[] = {
"24 24 4 1",
" \tc None",
".\tc #DF0000",
"+\tc #E00000",
"@\tc #DE0000",
"                        ",
"           ..           ",
"          ....          ",
"         .....+         ",
"        .......+        ",
"       ..........       ",
"      ...........@      ",
"     ..............     ",
"    ...............@    ",
"   ..................   ",
"  ...................+  ",
" ...................... ",
" ...................... ",
"  ...................+  ",
"   +.................   ",
"    ................    ",
"     ..............     ",
"      @..........+      ",
"       ..........       ",
"        ........        ",
"         .....+         ",
"          ....          ",
"           ..           ",
"                        "
};
static Fl_Pixmap image_diamonds(idata_diamonds);

void UserInterface::cb_hearts_i(Fl_Button*, void*) {
  f["game select"]();
}
void UserInterface::cb_hearts(Fl_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_hearts_i(o,v);
}

static const char *idata_hearts[] = {
"24 23 4 1",
" \tc None",
".\tc #E00000",
"+\tc #DF0000",
"@\tc #DE0000",
"                        ",
"   .+++++      ++++++   ",
"  ++++++++    ++++++++  ",
" ++++++++++  ++++++++++ ",
"+++++++++++@++++++++++++",
"++++++++++++++++++++++++",
"++++++++++++++++++++++++",
"++++++++++++++++++++++++",
"+++++++++++++++++++++++@",
" ++++++++++++++++++++++ ",
" ++++++++++++++++++++++ ",
"  ++++++++++++++++++++  ",
"  +++++++++++++++++++@  ",
"   .+++++++++++++++++   ",
"    ++++++++++++++++    ",
"     ++++++++++++++     ",
"      ++++++++++++      ",
"       ++++++++++       ",
"        .+++++++        ",
"         ++++++         ",
"          ++++          ",
"           ++           ",
"                        "
};
static Fl_Pixmap image_hearts(idata_hearts);

void UserInterface::cb_spades_i(Fl_Button*, void*) {
  f["game select"]();
}
void UserInterface::cb_spades(Fl_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_spades_i(o,v);
}

static const char *idata_spades[] = {
"24 28 2 1",
" \tc None",
".\tc #000000",
"                        ",
"           ..           ",
"          ....          ",
"         ......         ",
"        ........        ",
"       ..........       ",
"      ............      ",
"     ..............     ",
"    ................    ",
"   ..................   ",
"  ....................  ",
" ...................... ",
" ...................... ",
" ...................... ",
" ...................... ",
" ...................... ",
" ...................... ",
" ...................... ",
"  ........ .. ........  ",
"   .....   ..   .....   ",
"           ..           ",
"           ..           ",
"          ....          ",
"          ....          ",
"          ....          ",
"         ......         ",
"         ......         ",
"                        "
};
static Fl_Pixmap image_spades(idata_spades);

void UserInterface::cb_clubs_i(Fl_Button*, void*) {
  f["game select"]();
}
void UserInterface::cb_clubs(Fl_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_clubs_i(o,v);
}

static const char *idata_clubs[] = {
"26 24 2 1",
" \tc None",
".\tc #000000",
"           ....           ",
"         ........         ",
"         ........         ",
"        ..........        ",
"        ..........        ",
"         ........         ",
"         ........         ",
"          ......          ",
"   ...    ......    ...   ",
"  ......   ....   ......  ",
" ........   ..   ........ ",
"..........  ..  ..........",
"..........................",
"..........................",
"........... .. ...........",
" .........  ..  ......... ",
"  ......    ..    ......  ",
"   ...      ..      ...   ",
"            ..            ",
"           ....           ",
"           ....           ",
"           ....           ",
"          ......          ",
"          ......          "
};
static Fl_Pixmap image_clubs(idata_clubs);

void UserInterface::cb_grand_i(Fl_Button*, void*) {
  f["game select"]();
}
void UserInterface::cb_grand(Fl_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_grand_i(o,v);
}

void UserInterface::cb_null_i(Fl_Button*, void*) {
  f["game select"]();
}
void UserInterface::cb_null(Fl_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_null_i(o,v);
}

void UserInterface::cb_nullouvert_i(Fl_Button*, void*) {
  f["game select"]();
}
void UserInterface::cb_nullouvert(Fl_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_nullouvert_i(o,v);
}

void UserInterface::cb_schneider_i(Fl_Button*, void*) {
  if (!schneider->value()) {
  schwarz->value(false);
  ouvert->value(false);
}
f["game select"]();
}
void UserInterface::cb_schneider(Fl_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_schneider_i(o,v);
}

void UserInterface::cb_schwarz_i(Fl_Button*, void*) {
  if (schwarz->value())
  schneider->value(true);
else
  ouvert->value(false);
f["game select"]();
}
void UserInterface::cb_schwarz(Fl_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_schwarz_i(o,v);
}

void UserInterface::cb_ouvert_i(Fl_Button*, void*) {
  if (ouvert->value()) {
  schneider->value(true);
  schwarz->value(true);
}
f["game select"]();
}
void UserInterface::cb_ouvert(Fl_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_ouvert_i(o,v);
}

void UserInterface::cb_skat_i(Fl_Button*, void*) {
  f["skat take"]();
}
void UserInterface::cb_skat(Fl_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_skat_i(o,v);
}

void UserInterface::cb_announce_i(Fl_Button*, void*) {
  f["game announce"]();
}
void UserInterface::cb_announce(Fl_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_announce_i(o,v);
}

void UserInterface::cb_dealout_i(Fl_Button*, void*) {
  f["game dealout"]();
}
void UserInterface::cb_dealout(Fl_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_dealout_i(o,v);
}

void UserInterface::cb_disclose_i(Fl_Button*, void*) {
  f["game disclose"]();
}
void UserInterface::cb_disclose(Fl_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_disclose_i(o,v);
}

void UserInterface::cb_contrare_i(Fl_Button*, void*) {
  f["game contrare"]();
}
void UserInterface::cb_contrare(Fl_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_contrare_i(o,v);
}

void UserInterface::cb_giveup_i(Fl_Button*, void*) {
  f["game giveup"]();
}
void UserInterface::cb_giveup(Fl_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_giveup_i(o,v);
}

void UserInterface::cb_midimage_i(SK::GlImage*, void*) {
  f["audio mute"]();
}
void UserInterface::cb_midimage(SK::GlImage* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->user_data()))->cb_midimage_i(o,v);
}

void UserInterface::cb_chat_i(Fl_Input*, void*) {
  f["chat message"]();
}
void UserInterface::cb_chat(Fl_Input* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->user_data()))->cb_chat_i(o,v);
}

void UserInterface::cb_Restart_i(Fl_Button*, void*) {
  f["audio restart"]();
}
void UserInterface::cb_Restart(Fl_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_Restart_i(o,v);
}

void UserInterface::cb_Playback_i(Fl_Check_Button*, void*) {
  f["audio toggle"]();
}
void UserInterface::cb_Playback(Fl_Check_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_Playback_i(o,v);
}

void UserInterface::cb_address_i(Fl_Input*, void*) {
  prefs.set("ipaddress", address->value());
}
void UserInterface::cb_address(Fl_Input* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_address_i(o,v);
}

void UserInterface::cb_port_i(Fl_Value_Input*, void*) {
  if (port->value() < 0)
  port->value(0);
if (port->value() > 65535)
  port->value(65535);
prefs.set("udpport", port->value());
}
void UserInterface::cb_port(Fl_Value_Input* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_port_i(o,v);
}

void UserInterface::cb_bandwidth_i(Fl_Value_Input*, void*) {
  if (bandwidth->value() < 8000)
  bandwidth->value(8000);
prefs.set("bandwidth", bandwidth->value());
}
void UserInterface::cb_bandwidth(Fl_Value_Input* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_bandwidth_i(o,v);
}

void UserInterface::cb_Connect_i(Fl_Button*, void*) {
  f["network connect"]();
}
void UserInterface::cb_Connect(Fl_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_Connect_i(o,v);
}

void UserInterface::cb_Stats_i(Fl_Button*, void*) {
  f["network stats"]();
}
void UserInterface::cb_Stats(Fl_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_Stats_i(o,v);
}

void UserInterface::cb_server_i(Fl_Check_Button*, void*) {
  prefs.set("server", server->value());
}
void UserInterface::cb_server(Fl_Check_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_server_i(o,v);
}

void UserInterface::cb_name_i(Fl_Input*, void*) {
  prefs.set("username", name->value());
f["name change"]();
}
void UserInterface::cb_name(Fl_Input* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_name_i(o,v);
}

void UserInterface::cb_secret_i(Fl_Input*, void*) {
  prefs.set("secret", secret->value());
}
void UserInterface::cb_secret(Fl_Input* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_secret_i(o,v);
}

void UserInterface::cb_bgcolor_i(Fl_Button*, void*) {
  uchar r, g, b;
Fl::get_color(bgcolor->color(), r, g, b);
fl_color_chooser("Choose Color", r, g, b);

Fl_Color color(fl_rgb_color(r, g, b));
bgcolor->color(color);
bgcolor->redraw();
table->set_bgcolor(color);
prefs.set("bgcolor", (int)color);
}
void UserInterface::cb_bgcolor(Fl_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_bgcolor_i(o,v);
}

void UserInterface::cb_foldrule_i(Fl_Check_Button*, void*) {
  prefs.set("rulefold", foldrule->value());
f["rule change"]();
}
void UserInterface::cb_foldrule(Fl_Check_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_foldrule_i(o,v);
}

void UserInterface::cb_contrarerule_i(Fl_Check_Button*, void*) {
  prefs.set("rulecontrare", contrarerule->value());
f["rule change"]();
}
void UserInterface::cb_contrarerule(Fl_Check_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_contrarerule_i(o,v);
}

void UserInterface::cb_bockrule_i(Fl_Check_Button*, void*) {
  prefs.set("rulebock", bockrule->value());
f["rule change"]();
}
void UserInterface::cb_bockrule(Fl_Check_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_bockrule_i(o,v);
}

void UserInterface::cb_junkrule_i(Fl_Check_Button*, void*) {
  prefs.set("rulejunk", junkrule->value());
f["rule change"]();
}
void UserInterface::cb_junkrule(Fl_Check_Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_junkrule_i(o,v);
}

UserInterface::UserInterface():prefs(Fl_Preferences::USER, "cpaproth", "sk") {
  { mainwnd = new Fl_Double_Window(960, 700, "Skat-Konferenz");
    mainwnd->labelsize(11);
    mainwnd->callback((Fl_Callback*)cb_mainwnd, (void*)(this));
    mainwnd->align(Fl_Align(FL_ALIGN_CLIP|FL_ALIGN_INSIDE));
    { Fl_Tabs* o = new Fl_Tabs(0, 0, 960, 700);
      o->labelsize(11);
      { Fl_Group* o = new Fl_Group(0, 25, 960, 675, "Skat");
        o->labelsize(11);
        { table = new SK::GlTable(0, 265, 640, 435);
          table->box(FL_FLAT_BOX);
          table->color((Fl_Color)1908830464);
          table->selection_color(FL_BACKGROUND_COLOR);
          table->labeltype(FL_NORMAL_LABEL);
          table->labelfont(0);
          table->labelsize(11);
          table->labelcolor(FL_FOREGROUND_COLOR);
          table->callback((Fl_Callback*)cb_table);
          table->align(Fl_Align(FL_ALIGN_CENTER));
          table->when(FL_WHEN_RELEASE);
        } // SK::GlTable* table
        { Fl_Group* o = new Fl_Group(0, 25, 640, 240);
          { leftimage = new SK::GlImage(0, 25, 320, 240);
            leftimage->box(FL_FLAT_BOX);
            leftimage->color(FL_BLACK);
            leftimage->selection_color(FL_BACKGROUND_COLOR);
            leftimage->labeltype(FL_NORMAL_LABEL);
            leftimage->labelfont(0);
            leftimage->labelsize(11);
            leftimage->labelcolor(FL_FOREGROUND_COLOR);
            leftimage->align(Fl_Align(FL_ALIGN_CENTER));
            leftimage->when(FL_WHEN_RELEASE);
          } // SK::GlImage* leftimage
          { rightimage = new SK::GlImage(320, 25, 320, 240);
            rightimage->box(FL_FLAT_BOX);
            rightimage->color(FL_BLACK);
            rightimage->selection_color(FL_BACKGROUND_COLOR);
            rightimage->labeltype(FL_NORMAL_LABEL);
            rightimage->labelfont(0);
            rightimage->labelsize(11);
            rightimage->labelcolor(FL_FOREGROUND_COLOR);
            rightimage->align(Fl_Align(FL_ALIGN_CENTER));
            rightimage->when(FL_WHEN_RELEASE);
          } // SK::GlImage* rightimage
          o->end();
          Fl_Group::current()->resizable(o);
        } // Fl_Group* o
        { Fl_Group* o = new Fl_Group(640, 25, 320, 415);
          { Fl_Group* o = new Fl_Group(640, 25, 320, 85);
            o->box(FL_DOWN_BOX);
            o->labelsize(11);
            { info = new Fl_Box(640, 35, 320, 25, "Information");
              info->labelfont(1);
              info->labelsize(11);
              info->align(Fl_Align(FL_ALIGN_TEXT_OVER_IMAGE));
            } // Fl_Box* info
            { bid = new SK::BidButton(680, 70, 105, 25, "Reizen");
              bid->box(FL_PLASTIC_UP_BOX);
              bid->color((Fl_Color)2);
              bid->selection_color(FL_BACKGROUND_COLOR);
              bid->labeltype(FL_NORMAL_LABEL);
              bid->labelfont(1);
              bid->labelsize(11);
              bid->labelcolor(FL_FOREGROUND_COLOR);
              bid->callback((Fl_Callback*)cb_bid);
              bid->align(Fl_Align(FL_ALIGN_CENTER));
              bid->when(FL_WHEN_RELEASE);
            } // SK::BidButton* bid
            { fold = new Fl_Button(815, 70, 105, 25, "Passen");
              fold->box(FL_PLASTIC_UP_BOX);
              fold->color((Fl_Color)1);
              fold->labelfont(1);
              fold->labelsize(11);
              fold->callback((Fl_Callback*)cb_fold);
            } // Fl_Button* fold
            o->end();
          } // Fl_Group* o
          { Fl_Group* o = new Fl_Group(640, 110, 320, 240);
            o->box(FL_DOWN_BOX);
            o->labelsize(11);
            { gameinfo = new Fl_Box(640, 120, 320, 25, "Spielinfo");
              gameinfo->labelfont(1);
              gameinfo->labelsize(11);
              gameinfo->align(Fl_Align(FL_ALIGN_TEXT_OVER_IMAGE));
            } // Fl_Box* gameinfo
            { Fl_Group* o = new Fl_Group(700, 155, 200, 70);
              o->labelsize(11);
              { diamonds = new Fl_Button(700, 155, 50, 35);
                diamonds->tooltip("Karo ist Trumpf.");
                diamonds->type(102);
                diamonds->value(1);
                diamonds->image(image_diamonds);
                diamonds->labelsize(11);
                diamonds->callback((Fl_Callback*)cb_diamonds);
              } // Fl_Button* diamonds
              { hearts = new Fl_Button(750, 155, 50, 35);
                hearts->tooltip("Herz ist Trumpf.");
                hearts->type(102);
                hearts->image(image_hearts);
                hearts->labelsize(11);
                hearts->callback((Fl_Callback*)cb_hearts);
              } // Fl_Button* hearts
              { spades = new Fl_Button(800, 155, 50, 35);
                spades->tooltip("Pik ist Trumpf.");
                spades->type(102);
                spades->image(image_spades);
                spades->labelsize(11);
                spades->callback((Fl_Callback*)cb_spades);
              } // Fl_Button* spades
              { clubs = new Fl_Button(850, 155, 50, 35);
                clubs->tooltip("Kreuz ist Trumpf.");
                clubs->type(102);
                clubs->image(image_clubs);
                clubs->labelsize(11);
                clubs->callback((Fl_Callback*)cb_clubs);
              } // Fl_Button* clubs
              { grand = new Fl_Button(700, 190, 60, 35, "Grand");
                grand->tooltip("Nur Buben sind Trumpf.");
                grand->type(102);
                grand->labelfont(1);
                grand->labelsize(11);
                grand->callback((Fl_Callback*)cb_grand);
              } // Fl_Button* grand
              { null = new Fl_Button(760, 190, 45, 35, "Null");
                null->tooltip("Nullspiel, es gibt kein Trumpf.");
                null->type(102);
                null->labelfont(1);
                null->labelsize(11);
                null->callback((Fl_Callback*)cb_null);
              } // Fl_Button* null
              { nullouvert = new Fl_Button(805, 190, 95, 35, "Null Ouvert");
                nullouvert->tooltip("Offenes Nullspiel, es gibt kein Trumpf.");
                nullouvert->type(102);
                nullouvert->labelfont(1);
                nullouvert->labelsize(11);
                nullouvert->callback((Fl_Callback*)cb_nullouvert);
              } // Fl_Button* nullouvert
              o->end();
            } // Fl_Group* o
            { hand = new Fl_Group(700, 225, 200, 35);
              hand->labelsize(11);
              { schneider = new Fl_Button(700, 225, 80, 35, "Schneider");
                schneider->tooltip("Handspiel, Schneider angesagt.");
                schneider->type(1);
                schneider->labelfont(1);
                schneider->labelsize(11);
                schneider->callback((Fl_Callback*)cb_schneider);
              } // Fl_Button* schneider
              { schwarz = new Fl_Button(780, 225, 65, 35, "Schwarz");
                schwarz->tooltip("Handspiel, Schneider und Schwarz angesagt.");
                schwarz->type(1);
                schwarz->labelfont(1);
                schwarz->labelsize(11);
                schwarz->callback((Fl_Callback*)cb_schwarz);
              } // Fl_Button* schwarz
              { ouvert = new Fl_Button(845, 225, 55, 35, "Ouvert");
                ouvert->tooltip("Offenes Handspiel, Schneider und Schwarz angesagt.");
                ouvert->type(1);
                ouvert->labelfont(1);
                ouvert->labelsize(11);
                ouvert->callback((Fl_Callback*)cb_ouvert);
              } // Fl_Button* ouvert
              hand->end();
            } // Fl_Group* hand
            { skat = new Fl_Button(700, 270, 200, 25, "Skat aufnehmen");
              skat->tooltip("Kein Handspiel.");
              skat->labelfont(1);
              skat->labelsize(11);
              skat->callback((Fl_Callback*)cb_skat);
            } // Fl_Button* skat
            { announce = new Fl_Button(700, 305, 200, 25, "Spiel ansagen");
              announce->labelfont(1);
              announce->labelsize(11);
              announce->callback((Fl_Callback*)cb_announce);
            } // Fl_Button* announce
            o->end();
          } // Fl_Group* o
          { Fl_Group* o = new Fl_Group(640, 350, 320, 90);
            o->box(FL_DOWN_BOX);
            o->labelsize(11);
            { dealout = new Fl_Button(655, 365, 140, 25, "Karten austeilen");
              dealout->color((Fl_Color)2);
              dealout->labelfont(1);
              dealout->labelsize(11);
              dealout->callback((Fl_Callback*)cb_dealout);
            } // Fl_Button* dealout
            { disclose = new Fl_Button(655, 400, 140, 25, "Karten aufdecken");
              disclose->labelfont(1);
              disclose->labelsize(11);
              disclose->callback((Fl_Callback*)cb_disclose);
            } // Fl_Button* disclose
            { contrare = new Fl_Button(810, 365, 140, 25, "Kontra / Re");
              contrare->type(1);
              contrare->color((Fl_Color)1);
              contrare->labelfont(1);
              contrare->labelsize(11);
              contrare->callback((Fl_Callback*)cb_contrare);
            } // Fl_Button* contrare
            { giveup = new Fl_Button(810, 400, 140, 25, "Spiel abbrechen");
              giveup->type(1);
              giveup->labelfont(1);
              giveup->labelsize(11);
              giveup->callback((Fl_Callback*)cb_giveup);
            } // Fl_Button* giveup
            o->end();
          } // Fl_Group* o
          o->end();
        } // Fl_Group* o
        { midimage = new SK::GlImage(640, 440, 320, 240);
          midimage->box(FL_FLAT_BOX);
          midimage->color(FL_BLACK);
          midimage->selection_color(FL_BACKGROUND_COLOR);
          midimage->labeltype(FL_NORMAL_LABEL);
          midimage->labelfont(0);
          midimage->labelsize(11);
          midimage->labelcolor(FL_FOREGROUND_COLOR);
          midimage->callback((Fl_Callback*)cb_midimage);
          midimage->align(Fl_Align(FL_ALIGN_CENTER));
          midimage->when(FL_WHEN_RELEASE);
          midimage->set();
        } // SK::GlImage* midimage
        { chat = new Fl_Input(640, 680, 320, 20);
          chat->box(FL_FLAT_BOX);
          chat->labelsize(11);
          chat->textsize(11);
          chat->callback((Fl_Callback*)cb_chat);
          chat->when(FL_WHEN_ENTER_KEY);
        } // Fl_Input* chat
        o->end();
      } // Fl_Group* o
      { Fl_Group* o = new Fl_Group(0, 25, 960, 675, "System");
        o->labelsize(11);
        o->hide();
        { Fl_Group* o = new Fl_Group(630, 110, 220, 140, "Audio");
          o->box(FL_DOWN_BOX);
          o->labeltype(FL_ENGRAVED_LABEL);
          o->labelsize(11);
          o->align(Fl_Align(33));
          { Fl_Button* o = new Fl_Button(665, 150, 150, 25, "Restart Audio Stream");
            o->tooltip("Restart the audio stream and print the current CPU load of the audio stream i\
nto the log window.");
            o->labelsize(11);
            o->callback((Fl_Callback*)cb_Restart);
          } // Fl_Button* o
          { Fl_Check_Button* o = new Fl_Check_Button(665, 200, 150, 25, "Playback Microphone");
            o->tooltip("When activated, the microphone recording will not be broadcasted but directly\
 played back. Use this to adjust your mixer settings, e.g. to reduce echoes.");
            o->down_box(FL_DOWN_BOX);
            o->labelsize(11);
            o->callback((Fl_Callback*)cb_Playback);
          } // Fl_Check_Button* o
          o->end();
        } // Fl_Group* o
        { Fl_Group* o = new Fl_Group(65, 70, 355, 290, "Network");
          o->box(FL_DOWN_BOX);
          o->labeltype(FL_ENGRAVED_LABEL);
          o->labelsize(11);
          o->align(Fl_Align(33));
          { address = new Fl_Input(200, 110, 195, 25, "IP Address");
            address->tooltip("The IP address or hostname of the server you want to connect to. If you want \
to be the server (one of the peers has to be the server), tick the Server chec\
kbox and click Connect. Then tell the other peers your publicly reachable IP a\
ddress or hostname, and UDP port.");
            address->labelsize(11);
            address->textsize(11);
            address->callback((Fl_Callback*)cb_address);
            char* c; prefs.get("ipaddress", c, ""); address->value(c); delete[] c;
          } // Fl_Input* address
          { port = new Fl_Value_Input(200, 160, 195, 25, "UDP Port");
            port->tooltip("The UDP port of the server.");
            port->color((Fl_Color)-256);
            port->labelsize(11);
            port->maximum(65535);
            port->step(1);
            port->textsize(11);
            port->callback((Fl_Callback*)cb_port);
            port->when(FL_WHEN_RELEASE);
            double d; prefs.get("udpport", d, 34588); port->value(d < 0? 0: d > 65535? 65535: d);
          } // Fl_Value_Input* port
          { bandwidth = new Fl_Value_Input(200, 210, 195, 25, "Upload Bandwidth");
            bandwidth->tooltip("Maximal bandwidth for the upload in bytes per second. This value shouldn\'t b\
e greater than the upload speed of your internet connection. The bandwidth is \
distributed among the connnected peers, the audio stream always needs 2500 byt\
es per second per peer.");
            bandwidth->color((Fl_Color)-256);
            bandwidth->labelsize(11);
            bandwidth->minimum(8000);
            bandwidth->maximum(1e+09);
            bandwidth->step(1000);
            bandwidth->textsize(11);
            bandwidth->callback((Fl_Callback*)cb_bandwidth);
            bandwidth->when(FL_WHEN_RELEASE);
            double d; prefs.get("bandwidth", d, 16000); bandwidth->value(d < 8000? 8000: d);
          } // Fl_Value_Input* bandwidth
          { Fl_Button* o = new Fl_Button(200, 260, 195, 25, "Connect");
            o->tooltip("Connect to the server or run the server. If a connection can be established, \
the videoconferencing starts. If 3 peers are connected with each other, the ga\
me starts.");
            o->labelsize(11);
            o->callback((Fl_Callback*)cb_Connect);
          } // Fl_Button* o
          { Fl_Button* o = new Fl_Button(200, 310, 195, 25, "Stats");
            o->tooltip("Print some stats on the connected peers into the log window.");
            o->labelsize(11);
            o->callback((Fl_Callback*)cb_Stats);
          } // Fl_Button* o
          { server = new Fl_Check_Button(115, 260, 85, 25, "Server");
            server->tooltip("When activated, the program will run as the server after the Connect button w\
as pressed.");
            server->down_box(FL_DOWN_BOX);
            server->labelsize(11);
            server->callback((Fl_Callback*)cb_server);
            int i; prefs.get("server", i, 0); server->value(i != 0);
          } // Fl_Check_Button* server
          o->end();
        } // Fl_Group* o
        { Fl_Group* o = new Fl_Group(525, 320, 245, 175, "Player");
          o->box(FL_DOWN_BOX);
          o->labeltype(FL_ENGRAVED_LABEL);
          o->labelsize(11);
          o->align(Fl_Align(33));
          { name = new Fl_Input(580, 360, 165, 25, "Name");
            name->tooltip("Your name during the game.");
            name->labelsize(11);
            name->textsize(11);
            name->callback((Fl_Callback*)cb_name);
            char* c; prefs.get("username", c, "nobody"); name->value(c); delete[] c;
          } // Fl_Input* name
          { secret = new Fl_Input(580, 405, 165, 25, "Secret");
            secret->tooltip("The secret and the time are used to initialize your random number generator a\
t program start.");
            secret->labelsize(11);
            secret->textsize(11);
            secret->callback((Fl_Callback*)cb_secret);
            char* c; prefs.get("secret", c, ""); secret->value(c); delete[] c;
          } // Fl_Input* secret
          { bgcolor = new Fl_Button(580, 450, 165, 25, "Color");
            bgcolor->tooltip("Choose the color of the table background.");
            bgcolor->labelsize(11);
            bgcolor->callback((Fl_Callback*)cb_bgcolor);
            bgcolor->align(Fl_Align(FL_ALIGN_LEFT));
            int i; prefs.get("bgcolor", i, (int)FL_GRAY); bgcolor->color(i); table->set_bgcolor(i);
          } // Fl_Button* bgcolor
          o->end();
        } // Fl_Group* o
        { Fl_Group* o = new Fl_Group(170, 425, 275, 175, "Sonderregeln (SR)");
          o->tooltip("Es gelten die Regeln der internationalen Skatordnung. Es sind nur die Sonderr\
egeln wirksam, die bei allen Spielern aktiviert sind. Ramsch wird ohne Schiebe\
n und Grand Hand gespielt. Der Spieler mit den wenigsten Augen gewinnt das Ram\
sch-Spiel mit 23 Punkten, macht er gar keinen Stich (Jungfrau), gewinnt er mit\
 46 Punkten, au\303\237""er einem Spieler gelingt es alle Stiche zu bekommen (\
Durchmarsch), dann gewinnt dieser mit 120 Punkten.");
          o->box(FL_DOWN_BOX);
          o->labeltype(FL_ENGRAVED_LABEL);
          o->labelsize(11);
          o->align(Fl_Align(33));
          { foldrule = new Fl_Check_Button(195, 465, 210, 25, "Ramschen statt Einpassen (E)");
            foldrule->tooltip("Wenn alle Spieler passen, wird mit der Hand Ramsch ohne Schieben gespielt. De\
r Spieler mit den wenigsten Augen bekommt ein gewonnenes Nullspiel gutgeschrie\
ben.");
            foldrule->down_box(FL_DOWN_BOX);
            foldrule->labelsize(11);
            foldrule->callback((Fl_Callback*)cb_foldrule);
            int i; prefs.get("rulefold", i, 0); foldrule->value(i != 0);
          } // Fl_Check_Button* foldrule
          { contrarerule = new Fl_Check_Button(195, 495, 135, 25, "Kontra und Re (K)");
            contrarerule->tooltip("Gegenspieler, die nicht bei 18 gepasst haben, d\303\274rfen bis zur 4. ausges\
pielten Karte Kontra sagen. Der Alleinspieler darf daraufhin bis zur 7. ausges\
pielten Karte Re sagen. Kontra und Re verdoppeln jeweils die Punktzahl des Spi\
els.");
            contrarerule->down_box(FL_DOWN_BOX);
            contrarerule->labelsize(11);
            contrarerule->callback((Fl_Callback*)cb_contrarerule);
            int i; prefs.get("rulecontrare", i, 0); contrarerule->value(i != 0);
          } // Fl_Check_Button* contrarerule
          { bockrule = new Fl_Check_Button(195, 525, 115, 25, "Bockrunde (B)");
            bockrule->tooltip("In einer Bockrunde werden die Punkte jedes Spiels verdoppelt. Eine Bockrunde \
wird gespielt nach verlorenem Kontra-Spiel, Kontra-Re-Spiel, Spiel mit 60 zu 6\
0 Augen oder gewonnenem Spiel mit wenigstens 100 Punkten Grundwert.");
            bockrule->down_box(FL_DOWN_BOX);
            bockrule->labelsize(11);
            bockrule->callback((Fl_Callback*)cb_bockrule);
            int i; prefs.get("rulebock", i, 0); bockrule->value(i != 0);
          } // Fl_Check_Button* bockrule
          { junkrule = new Fl_Check_Button(195, 555, 135, 25, "Ramschrunde (R)");
            junkrule->tooltip("Eine Ramschrunde wird unter den gleichen Bedingungen wie eine Bockrunde ausge\
l\303\266st. Auch hier werden die Punkte der Spiele verdoppelt, allerdings ist\
 jedes Spiel Ramsch. Wenn Bock- und Ramschrunde gespielt werden soll, dann fol\
gt die Ramsch- auf die Bockrunde.");
            junkrule->down_box(FL_DOWN_BOX);
            junkrule->labelsize(11);
            junkrule->callback((Fl_Callback*)cb_junkrule);
            int i; prefs.get("rulejunk", i, 0); junkrule->value(i != 0);
          } // Fl_Check_Button* junkrule
          o->end();
        } // Fl_Group* o
        o->end();
      } // Fl_Group* o
      { Fl_Group* o = new Fl_Group(0, 25, 960, 675, "Log");
        o->labelsize(11);
        o->hide();
        { SK::LogDisplay* o = new SK::LogDisplay(0, 25, 480, 675);
          o->box(FL_DOWN_BOX);
          o->color((Fl_Color)55);
          o->selection_color(FL_SELECTION_COLOR);
          o->labeltype(FL_NORMAL_LABEL);
          o->labelfont(0);
          o->labelsize(11);
          o->labelcolor(FL_FOREGROUND_COLOR);
          o->textsize(11);
          o->align(Fl_Align(FL_ALIGN_TOP));
          o->when(FL_WHEN_RELEASE);
          Fl_Group::current()->resizable(o);
        } // SK::LogDisplay* o
        { listing = new Fl_Browser(480, 25, 480, 675);
          listing->selection_color((Fl_Color)215);
          listing->labelsize(11);
          listing->textsize(9);
          static const int widths[] = {130, 50, 80, 80, 80, 60, 0}; listing->column_widths(widths);
          listing->add("@B49@cSpiel\t@B49@cPunkte\t@B49@cDu\t@B49@cLinks\t@B49@cRechts\t@B49@cSR");
        } // Fl_Browser* listing
        o->end();
        Fl_Group::current()->resizable(o);
      } // Fl_Group* o
      o->end();
    } // Fl_Tabs* o
    mainwnd->size_range(960, 700);
    mainwnd->end();
    mainwnd->resizable(mainwnd);
  } // Fl_Double_Window* mainwnd
  mainwnd->show();
  chat->take_focus();
}

UserInterface::~UserInterface() {
  delete mainwnd;
}
