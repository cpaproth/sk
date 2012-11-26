// generated by Fast Light User Interface Designer (fluid) version 2.1000

#include "ui.h"
//Copyright (C) 2012 Carsten Paproth
using namespace SK;
using namespace images;

inline void UserInterface::cb_mainwnd_i(fltk::Window*, void*) {
  mainwnd->hide();
}
void UserInterface::cb_mainwnd(fltk::Window* o, void* v) {
  ((UserInterface*)(o->user_data()))->cb_mainwnd_i(o,v);
}

inline void UserInterface::cb_table_i(GlTable*, void*) {
  f["table event"]();
}
void UserInterface::cb_table(GlTable* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->user_data()))->cb_table_i(o,v);
}

inline void UserInterface::cb_bid_i(BidButton*, void*) {
  f["game bid"]();
}
void UserInterface::cb_bid(BidButton* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_bid_i(o,v);
}

inline void UserInterface::cb_fold_i(fltk::Button*, void*) {
  f["game fold"]();
}
void UserInterface::cb_fold(fltk::Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_fold_i(o,v);
}

inline void UserInterface::cb_diamonds_i(fltk::Button*, void*) {
  f["game select"]();
}
void UserInterface::cb_diamonds(fltk::Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_diamonds_i(o,v);
}

inline void UserInterface::cb_hearts_i(fltk::Button*, void*) {
  f["game select"]();
}
void UserInterface::cb_hearts(fltk::Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_hearts_i(o,v);
}

inline void UserInterface::cb_spades_i(fltk::Button*, void*) {
  f["game select"]();
}
void UserInterface::cb_spades(fltk::Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_spades_i(o,v);
}

inline void UserInterface::cb_clubs_i(fltk::Button*, void*) {
  f["game select"]();
}
void UserInterface::cb_clubs(fltk::Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_clubs_i(o,v);
}

inline void UserInterface::cb_grand_i(fltk::Button*, void*) {
  f["game select"]();
}
void UserInterface::cb_grand(fltk::Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_grand_i(o,v);
}

inline void UserInterface::cb_null_i(fltk::Button*, void*) {
  f["game select"]();
}
void UserInterface::cb_null(fltk::Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_null_i(o,v);
}

inline void UserInterface::cb_nullouvert_i(fltk::Button*, void*) {
  f["game select"]();
}
void UserInterface::cb_nullouvert(fltk::Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_nullouvert_i(o,v);
}

inline void UserInterface::cb_schneider_i(fltk::Button*, void*) {
  if (!schneider->value()) {
    schwarz->value(false);
    ouvert->value(false);
  }
  f["game select"]();
}
void UserInterface::cb_schneider(fltk::Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_schneider_i(o,v);
}

inline void UserInterface::cb_schwarz_i(fltk::Button*, void*) {
  if (schwarz->value())
    schneider->value(true);
  else
    ouvert->value(false);
  f["game select"]();
}
void UserInterface::cb_schwarz(fltk::Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_schwarz_i(o,v);
}

inline void UserInterface::cb_ouvert_i(fltk::Button*, void*) {
  if (ouvert->value()) {
    schneider->value(true);
    schwarz->value(true);
  }
  f["game select"]();
}
void UserInterface::cb_ouvert(fltk::Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_ouvert_i(o,v);
}

inline void UserInterface::cb_skat_i(fltk::Button*, void*) {
  f["skat take"]();
}
void UserInterface::cb_skat(fltk::Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_skat_i(o,v);
}

inline void UserInterface::cb_announce_i(fltk::Button*, void*) {
  f["game announce"]();
}
void UserInterface::cb_announce(fltk::Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_announce_i(o,v);
}

inline void UserInterface::cb_Austeilen_i(fltk::Button*, void*) {
  f["dealing start"]();
}
void UserInterface::cb_Austeilen(fltk::Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_Austeilen_i(o,v);
}

inline void UserInterface::cb_Neustart_i(fltk::Button*, void*) {
  f["audio restart"]();
}
void UserInterface::cb_Neustart(fltk::Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_Neustart_i(o,v);
}

inline void UserInterface::cb_Mikrofon_i(fltk::CheckButton*, void*) {
  f["audio toggle"]();
}
void UserInterface::cb_Mikrofon(fltk::CheckButton* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_Mikrofon_i(o,v);
}

inline void UserInterface::cb_address_i(fltk::Input*, void*) {
  prefs.set("ipaddress", address->value());
}
void UserInterface::cb_address(fltk::Input* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_address_i(o,v);
}

inline void UserInterface::cb_port_i(fltk::ValueInput*, void*) {
  if (port->value() < 0)
    port->value(0);
  if (port->value() > 65535)
    port->value(65535);
  prefs.set("udpport", port->value());
}
void UserInterface::cb_port(fltk::ValueInput* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_port_i(o,v);
}

inline void UserInterface::cb_bandwidth_i(fltk::ValueInput*, void*) {
  if (bandwidth->value() < 0)
    bandwidth->value(0);
  prefs.set("bandwidth", bandwidth->value());
}
void UserInterface::cb_bandwidth(fltk::ValueInput* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_bandwidth_i(o,v);
}

inline void UserInterface::cb_Verbinden_i(fltk::Button*, void*) {
  f["network connect"]();
}
void UserInterface::cb_Verbinden(fltk::Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_Verbinden_i(o,v);
}

inline void UserInterface::cb_Stats_i(fltk::Button*, void*) {
  f["network stats"]();
}
void UserInterface::cb_Stats(fltk::Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_Stats_i(o,v);
}

inline void UserInterface::cb_autoconnect_i(fltk::CheckButton*, void*) {
  prefs.set("autoconnect", autoconnect->value());
}
void UserInterface::cb_autoconnect(fltk::CheckButton* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_autoconnect_i(o,v);
}

inline void UserInterface::cb_name_i(fltk::Input*, void*) {
  prefs.set("username", name->value());
  f["name change"]();
}
void UserInterface::cb_name(fltk::Input* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_name_i(o,v);
}

inline void UserInterface::cb_secret_i(fltk::Input*, void*) {
  prefs.set("secret", secret->value());
}
void UserInterface::cb_secret(fltk::Input* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_secret_i(o,v);
}
#include "../images/diamonds.xpm"
#include "../images/hearts.xpm"
#include "../images/spades.xpm"
#include "../images/clubs.xpm"
static fltk::xpmImage dia(diamonds_xpm, "dia");
static fltk::xpmImage hea(hearts_xpm, "hea");
static fltk::xpmImage spa(spades_xpm, "spa");
static fltk::xpmImage clu(clubs_xpm, "clu");

UILock::UILock() {
  fltk::lock();
}

UILock::~UILock() {
  fltk::unlock();
}

UserInterface::UserInterface(void):prefs(fltk::Preferences::USER, "cpaproth", "sk") {
  fltk::Window* w;
  UILock lock;
   {fltk::Window* o = mainwnd = new fltk::Window(960, 700, "Skat-Konferenz");
    w = o;
    o->callback((fltk::Callback*)cb_mainwnd, (void*)(this));
    o->begin();
     {fltk::TabGroup* o = new fltk::TabGroup(0, 0, 960, 700);
      o->begin();
       {fltk::Group* o = new fltk::Group(0, 25, 960, 675, "Spiel");
        o->begin();
         {GlTable* o = table = new GlTable(0, 240, 640, 435);
          o->box(fltk::FLAT_BOX);
          o->color((fltk::Color)0x71c67100);
          o->callback((fltk::Callback*)cb_table);
        }
         {GlImage* o = leftimage = new GlImage(0, 0, 320, 240);
          o->box(fltk::FLAT_BOX);
          o->color((fltk::Color)56);
        }
         {GlImage* o = rightimage = new GlImage(320, 0, 320, 240);
          o->box(fltk::FLAT_BOX);
          o->color((fltk::Color)56);
        }
         {GlImage* o = midimage = new GlImage(640, 435, 320, 240);
          o->box(fltk::FLAT_BOX);
          o->color((fltk::Color)56);
        }
         {fltk::Group* o = new fltk::Group(640, 0, 320, 110);
          o->box(fltk::DOWN_BOX);
          o->begin();
           {BidButton* o = bid = new BidButton(40, 60, 105, 25, "Reizen");
            o->buttonbox(fltk::PLASTIC_UP_BOX);
            o->labelfont(fltk::HELVETICA_BOLD);
            o->buttoncolor((fltk::Color)0xff0000);
            o->callback((fltk::Callback*)cb_bid);
          }
           {fltk::Button* o = fold = new fltk::Button(175, 60, 105, 25, "Passen");
            o->buttonbox(fltk::PLASTIC_UP_BOX);
            o->labelfont(fltk::HELVETICA_BOLD);
            o->buttoncolor((fltk::Color)0xff000000);
            o->callback((fltk::Callback*)cb_fold);
          }
           {fltk::InvisibleBox* o = info = new fltk::InvisibleBox(0, 20, 320, 25, "Information");
            o->labelfont(fltk::HELVETICA_BOLD);
            o->align(fltk::ALIGN_INSIDE);
          }
          o->end();
        }
         {fltk::Group* o = new fltk::Group(640, 110, 320, 240);
          o->box(fltk::DOWN_BOX);
          o->when(fltk::WHEN_NEVER);
          o->begin();
           {fltk::InvisibleBox* o = gameinfo = new fltk::InvisibleBox(0, 10, 320, 25, "Spielinfo");
            o->labelfont(fltk::HELVETICA_BOLD);
            o->align(fltk::ALIGN_INSIDE);
          }
           {fltk::Group* o = trump = new fltk::Group(60, 45, 200, 70);
            o->when(fltk::WHEN_NEVER);
            o->begin();
             {fltk::Button* o = diamonds = new fltk::Button(0, 0, 50, 35, "@dia");
              o->type(fltk::Button::RADIO);
              o->set_flag(fltk::STATE);
              o->callback((fltk::Callback*)cb_diamonds);
              o->tooltip("Karo ist Trumpf.");
            }
             {fltk::Button* o = hearts = new fltk::Button(50, 0, 50, 35, "@hea");
              o->type(fltk::Button::RADIO);
              o->callback((fltk::Callback*)cb_hearts);
              o->tooltip("Herz ist Trumpf.");
            }
             {fltk::Button* o = spades = new fltk::Button(100, 0, 50, 35, "@spa");
              o->type(fltk::Button::RADIO);
              o->callback((fltk::Callback*)cb_spades);
              o->tooltip("Pik ist Trumpf.");
            }
             {fltk::Button* o = clubs = new fltk::Button(150, 0, 50, 35, "@clu");
              o->type(fltk::Button::RADIO);
              o->callback((fltk::Callback*)cb_clubs);
              o->tooltip("Kreuz ist Trumpf.");
            }
             {fltk::Button* o = grand = new fltk::Button(0, 35, 60, 35, "Grand");
              o->type(fltk::Button::RADIO);
              o->labelfont(fltk::HELVETICA_BOLD);
              o->callback((fltk::Callback*)cb_grand);
              o->tooltip("Nur Buben sind Trumpf.");
            }
             {fltk::Button* o = null = new fltk::Button(60, 35, 45, 35, "Null");
              o->type(fltk::Button::RADIO);
              o->labelfont(fltk::HELVETICA_BOLD);
              o->callback((fltk::Callback*)cb_null);
              o->tooltip("Nullspiel, es gibt kein Trumpf.");
            }
             {fltk::Button* o = nullouvert = new fltk::Button(105, 35, 95, 35, "Null Ouvert");
              o->type(fltk::Button::RADIO);
              o->labelfont(fltk::HELVETICA_BOLD);
              o->callback((fltk::Callback*)cb_nullouvert);
              o->tooltip("Offenes Nullspiel, es gibt kein Trumpf.");
            }
            o->end();
          }
           {fltk::Group* o = hand = new fltk::Group(60, 115, 200, 35);
            o->when(fltk::WHEN_NEVER);
            o->begin();
             {fltk::Button* o = schneider = new fltk::Button(0, 0, 80, 35, "Schneider");
              o->type(fltk::Button::TOGGLE);
              o->labelfont(fltk::HELVETICA_BOLD);
              o->callback((fltk::Callback*)cb_schneider);
              o->tooltip("Handspiel, Schneider angesagt.");
            }
             {fltk::Button* o = schwarz = new fltk::Button(80, 0, 65, 35, "Schwarz");
              o->type(fltk::Button::TOGGLE);
              o->labelfont(fltk::HELVETICA_BOLD);
              o->callback((fltk::Callback*)cb_schwarz);
              o->tooltip("Handspiel, Schneider und Schwarz angesagt.");
            }
             {fltk::Button* o = ouvert = new fltk::Button(145, 0, 55, 35, "Ouvert");
              o->type(fltk::Button::TOGGLE);
              o->labelfont(fltk::HELVETICA_BOLD);
              o->callback((fltk::Callback*)cb_ouvert);
              o->tooltip("Offenes Handspiel, Schneider und Schwarz angesagt.");
            }
            o->end();
          }
           {fltk::Button* o = skat = new fltk::Button(60, 160, 200, 25, "Skat aufnehmen");
            o->labelfont(fltk::HELVETICA_BOLD);
            o->callback((fltk::Callback*)cb_skat);
            o->tooltip("Kein Handspiel.");
          }
           {fltk::Button* o = announce = new fltk::Button(60, 195, 200, 25, "Spiel ansagen");
            o->labelfont(fltk::HELVETICA_BOLD);
            o->callback((fltk::Callback*)cb_announce);
          }
          o->end();
        }
         {fltk::Group* o = new fltk::Group(640, 350, 320, 85);
          o->box(fltk::DOWN_BOX);
          o->begin();
           {fltk::Button* o = new fltk::Button(105, 30, 105, 25, "Austeilen");
            o->callback((fltk::Callback*)cb_Austeilen);
          }
          o->end();
        }
        o->end();
      }
       {fltk::Group* o = new fltk::Group(0, 25, 960, 675, "Liste");
        o->hide();
      }
       {fltk::Group* o = new fltk::Group(0, 25, 960, 675, "System");
        o->hide();
        o->begin();
         {fltk::Group* o = new fltk::Group(630, 85, 220, 145, "Audio");
          o->box(fltk::DOWN_BOX);
          o->labeltype(fltk::ENGRAVED_LABEL);
          o->align(fltk::ALIGN_TOP|fltk::ALIGN_INSIDE);
          o->begin();
           {fltk::Button* o = new fltk::Button(35, 40, 150, 25, "Neustart Audiostream");
            o->callback((fltk::Callback*)cb_Neustart);
            o->tooltip("Falls der Audiostream verz\303\266gert l\303\244uft, kann ein Neustart dies e\
vtl. beheben. Zeigt zus\303\244tzlich die gegenw\303\244rtige CPU Auslastung d\
es Audiostreams im Log-Fenster an.");
          }
           {fltk::CheckButton* o = new fltk::CheckButton(35, 90, 150, 25, "Mikrofon abspielen");
            o->callback((fltk::Callback*)cb_Mikrofon);
            o->tooltip("Wenn aktiviert, dann wird die Mikrofonaufnahme nicht \303\274""bers Netzwerk \
gesendet, sondern direkt abgespielt. Kann n\303\274tzlich sein um den Mixer ri\
chtig einstellen zu k\303\266nnen, damit z.B. Echos verringert werden. ");
          }
          o->end();
        }
         {fltk::Group* o = new fltk::Group(90, 60, 390, 290, "Netzwerk");
          o->box(fltk::DOWN_BOX);
          o->labeltype(fltk::ENGRAVED_LABEL);
          o->align(fltk::ALIGN_TOP|fltk::ALIGN_INSIDE);
          o->begin();
           {fltk::Input* o = address = new fltk::Input(165, 40, 195, 25, "IP-Adresse");
            o->callback((fltk::Callback*)cb_address);
            o->tooltip("Das Feld leer lassen, um beim Verbinden des Netzwerks als Server zu dienen. D\
amit sich andere Spieler als Clients mit diesem Server verbinden k\303\266nnen\
, musst du ihnen die \303\266""ffentlich erreichbare IP-Adresse und den ausgew\
\303\244hlten UDP-Port mitteilen.");
            char* c;
            prefs.get("ipaddress", c, "");
            address->value(c);
            delete[] c;
          }
           {fltk::ValueInput* o = port = new fltk::ValueInput(165, 90, 195, 25, "UDP-Port");
            o->color((fltk::Color)0xffffff00);
            o->maximum(65535);
            o->step(1);
            o->callback((fltk::Callback*)cb_port);
            o->when(fltk::WHEN_RELEASE);
            o->tooltip("Um einen Server mit einer Portnummer kleiner als 1024 zu starten, ben\303\
\266tigt man evtl. Root-Rechte.");
            double d;
            prefs.get("udpport", d, 34588);
            port->value(d);
          }
           {fltk::ValueInput* o = bandwidth = new fltk::ValueInput(165, 140, 195, 25, "Video Upload-Bandbreite");
            o->color((fltk::Color)0xffffff00);
            o->maximum(1e+09);
            o->step(1);
            o->callback((fltk::Callback*)cb_bandwidth);
            o->when(fltk::WHEN_RELEASE);
            o->tooltip("Maximale Upload-Bandbreite f\303\274r Video in Byte/s, wird auf die verbunden\
en Peers aufgeteilt. Der Audiostream ben\303\266tigt pro Peer immer 2500 Byte/\
s, sonstiger Netzwerkverkehr ist weniger als 1000 Byte/s pro Peer.");
            double d;
            prefs.get("bandwidth", d, 50000);
            bandwidth->value(d);
            bandwidth->linesize(1000);
          }
           {fltk::Button* o = new fltk::Button(165, 190, 195, 25, "Verbinden");
            o->callback((fltk::Callback*)cb_Verbinden);
            o->tooltip("Wenn das IP-Adressenfeld leer ist, dann startest du hiermit die Skat-Konferen\
z als Server, ansonsten wird eine Verbindung zu der angegebenen IP-Adresse und\
 UDP-Port aufgebaut. Wenn die Verbindung erfolgreich ist, dann beginnt die Vid\
eokonferenz. Wenn 3 Peers miteinander verbunden sind, dann startet das Spiel.");
          }
           {fltk::Button* o = new fltk::Button(165, 240, 195, 25, "Stats");
            o->callback((fltk::Callback*)cb_Stats);
            o->tooltip("Gibt ein paar Statistiken \303\274""ber die verbundenen Peers im Log-Fenster \
aus.");
          }
           {fltk::CheckButton* o = autoconnect = new fltk::CheckButton(70, 190, 60, 25, "Auto");
            o->callback((fltk::Callback*)cb_autoconnect);
            o->tooltip("Wenn aktiviert, dann wird das Netzwerk beim n\303\244""chsten Programmstart a\
utomatisch verbunden. Nur sinnvoll beim Start als Server oder bei Verbindung z\
u statischer IP-Adresse.");
            int i;
            prefs.get("autoconnect", i, 0);
            autoconnect->value(i != 0);
          }
          o->end();
        }
         {fltk::Group* o = new fltk::Group(525, 295, 265, 150, "Spiel");
          o->box(fltk::DOWN_BOX);
          o->labeltype(fltk::ENGRAVED_LABEL);
          o->align(fltk::ALIGN_TOP|fltk::ALIGN_INSIDE);
          o->begin();
           {fltk::Input* o = name = new fltk::Input(85, 40, 165, 25, "Spielername");
            o->callback((fltk::Callback*)cb_name);
            o->tooltip("Dein Spielername.");
            char* c;
            prefs.get("username", c, "nobody");
            name->value(c);
            delete[] c;
          }
           {fltk::Input* o = secret = new fltk::Input(85, 85, 165, 25, "Geheimnis");
            o->callback((fltk::Callback*)cb_secret);
            o->tooltip("Wird zus\303\244tzlich zur Zeit verwendet um den Zufallsgenerator zu initiali\
sieren.");
            char* c;
            prefs.get("secret", c, "");
            secret->value(c);
            delete[] c;
          }
          o->end();
        }
        o->end();
      }
       {fltk::Group* o = new fltk::Group(0, 25, 960, 675, "Log");
        o->hide();
        o->begin();
        new LogDisplay(0, 0, 960, 675);
        o->end();
      }
      o->end();
    }
    o->end();
    o->resizable(o);
  }
  w->resizable(0);
  w->show();
}

UserInterface::~UserInterface(void) {
  delete mainwnd;
}

UserInterface::UserInterface(const UserInterface&):prefs("", "", "") {
}

void UserInterface::operator=(const UserInterface&) {
}
