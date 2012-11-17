// generated by Fast Light User Interface Designer (fluid) version 2.1000

#include "ui.h"
//Copyright (C) 2012 Carsten Paproth
using namespace SK;
using namespace SK::images;

inline void UserInterface::cb_mainwnd_i(fltk::Window*, void*) {
  mainwnd->hide();
}
void UserInterface::cb_mainwnd(fltk::Window* o, void* v) {
  ((UserInterface*)(o->user_data()))->cb_mainwnd_i(o,v);
}

inline void UserInterface::cb_trump_i(fltk::Group*, void*) {
  if (null->value() || nullouvert->value()) {
    extra->deactivate();
    extra->do_callback();
  } else if (extra->user_data() == 0)
    extra->activate();
}
void UserInterface::cb_trump(fltk::Group* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_trump_i(o,v);
}

inline void UserInterface::cb_diamonds_i(fltk::RadioButton*, void*) {
  trump->do_callback();
}
void UserInterface::cb_diamonds(fltk::RadioButton* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_diamonds_i(o,v);
}

inline void UserInterface::cb_hearts_i(fltk::RadioButton*, void*) {
  trump->do_callback();
}
void UserInterface::cb_hearts(fltk::RadioButton* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_hearts_i(o,v);
}

inline void UserInterface::cb_spades_i(fltk::RadioButton*, void*) {
  trump->do_callback();
}
void UserInterface::cb_spades(fltk::RadioButton* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_spades_i(o,v);
}

inline void UserInterface::cb_clubs_i(fltk::RadioButton*, void*) {
  trump->do_callback();
}
void UserInterface::cb_clubs(fltk::RadioButton* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_clubs_i(o,v);
}

inline void UserInterface::cb_grand_i(fltk::RadioButton*, void*) {
  trump->do_callback();
}
void UserInterface::cb_grand(fltk::RadioButton* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_grand_i(o,v);
}

inline void UserInterface::cb_null_i(fltk::RadioButton*, void*) {
  trump->do_callback();
}
void UserInterface::cb_null(fltk::RadioButton* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_null_i(o,v);
}

inline void UserInterface::cb_nullouvert_i(fltk::RadioButton*, void*) {
  trump->do_callback();
}
void UserInterface::cb_nullouvert(fltk::RadioButton* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_nullouvert_i(o,v);
}

inline void UserInterface::cb_hand_i(fltk::Button*, void*) {
  extra->deactivate();
  extra->do_callback();
  extra->user_data((void*)1);
  hand->deactivate();
}
void UserInterface::cb_hand(fltk::Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_hand_i(o,v);
}

inline void UserInterface::cb_extra_i(fltk::Group*, void*) {
  schneider->value(false);
  schwarz->value(false);
  ouvert->value(false);
}
void UserInterface::cb_extra(fltk::Group* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_extra_i(o,v);
}

inline void UserInterface::cb_schneider_i(fltk::CheckButton*, void*) {
  if (!schneider->value()) {
    schwarz->value(false);
    ouvert->value(false);
  }
;}
void UserInterface::cb_schneider(fltk::CheckButton* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_schneider_i(o,v);
}

inline void UserInterface::cb_schwarz_i(fltk::CheckButton*, void*) {
  if (schwarz->value())
    schneider->value(true);
  else
    ouvert->value(false);
}
void UserInterface::cb_schwarz(fltk::CheckButton* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_schwarz_i(o,v);
}

inline void UserInterface::cb_ouvert_i(fltk::CheckButton*, void*) {
  if (ouvert->value()) {
    schwarz->value(true);
    schneider->value(true);
  }
;}
void UserInterface::cb_ouvert(fltk::CheckButton* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_ouvert_i(o,v);
}

inline void UserInterface::cb_play_i(fltk::Button*, void*) {
  trump->deactivate();
  hand->deactivate();
  extra->deactivate();
  play->deactivate();
}
void UserInterface::cb_play(fltk::Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_play_i(o,v);
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

inline void UserInterface::cb_Start_i(fltk::Button*, void*) {
  f["network start"]();
}
void UserInterface::cb_Start(fltk::Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_Start_i(o,v);
}

inline void UserInterface::cb_Stats_i(fltk::Button*, void*) {
  f["network stats"]();
}
void UserInterface::cb_Stats(fltk::Button* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_Stats_i(o,v);
}

inline void UserInterface::cb_autostart_i(fltk::CheckButton*, void*) {
  prefs.set("autostart", autostart->value());
}
void UserInterface::cb_autostart(fltk::CheckButton* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_autostart_i(o,v);
}

inline void UserInterface::cb_name_i(fltk::Input*, void*) {
  prefs.set("username", name->value());
}
void UserInterface::cb_name(fltk::Input* o, void* v) {
  ((UserInterface*)(o->parent()->parent()->parent()->parent()->user_data()))->cb_name_i(o,v);
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

UIUnlock::UIUnlock() {
  fltk::unlock();
}

UIUnlock::~UIUnlock() {
  fltk::lock();
}

UserInterface::UserInterface(void):prefs(fltk::Preferences::USER, "cpaproth", "sk") {
  fltk::Window* w;
  UILock lock;
   {fltk::Window* o = mainwnd = new fltk::Window(960, 700, "Skat-Konferenz");
    w = o;
    o->shortcut(0xff1b);
    o->callback((fltk::Callback*)cb_mainwnd, (void*)(this));
    o->begin();
     {fltk::TabGroup* o = new fltk::TabGroup(0, 0, 960, 700);
      o->begin();
       {fltk::Group* o = new fltk::Group(0, 25, 960, 675, "Spiel");
        o->begin();
         {GlTable* o = table = new GlTable(0, 240, 640, 435);
          o->box(fltk::FLAT_BOX);
          o->color((fltk::Color)0x71c67100);
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
         {fltk::Group* o = new fltk::Group(640, 0, 320, 435);
          o->set_vertical();
          o->box(fltk::DOWN_BOX);
          o->begin();
           {fltk::Group* o = trump = new fltk::Group(45, 20, 225, 150);
            o->box(fltk::DOWN_BOX);
            o->callback((fltk::Callback*)cb_trump);
            o->when(fltk::WHEN_NEVER);
            o->tooltip("Trumpf");
            o->begin();
             {fltk::RadioButton* o = diamonds = new fltk::RadioButton(35, 5, 65, 35, "@dia");
              o->set_flag(fltk::STATE);
              o->callback((fltk::Callback*)cb_diamonds);
              o->tooltip("Karo ist Trumpf.");
            }
             {fltk::RadioButton* o = hearts = new fltk::RadioButton(35, 40, 65, 35, "@hea");
              o->callback((fltk::Callback*)cb_hearts);
              o->tooltip("Herz ist Trumpf.");
            }
             {fltk::RadioButton* o = spades = new fltk::RadioButton(35, 75, 65, 35, "@spa");
              o->callback((fltk::Callback*)cb_spades);
              o->tooltip("Pik ist Trumpf.");
            }
             {fltk::RadioButton* o = clubs = new fltk::RadioButton(35, 110, 65, 35, "@clu");
              o->callback((fltk::Callback*)cb_clubs);
              o->tooltip("Kreuz ist Trumpf.");
            }
             {fltk::RadioButton* o = grand = new fltk::RadioButton(110, 5, 65, 35, "Grand");
              o->labelfont(fltk::HELVETICA_BOLD);
              o->callback((fltk::Callback*)cb_grand);
              o->tooltip("Buben sind Trumpf.");
            }
             {fltk::RadioButton* o = null = new fltk::RadioButton(110, 60, 50, 35, "Null");
              o->labelfont(fltk::HELVETICA_BOLD);
              o->callback((fltk::Callback*)cb_null);
              o->tooltip("Nullspiel, es gibt kein Trumpf.");
            }
             {fltk::RadioButton* o = nullouvert = new fltk::RadioButton(110, 110, 110, 35, "Null Ouvert");
              o->labelfont(fltk::HELVETICA_BOLD);
              o->callback((fltk::Callback*)cb_nullouvert);
              o->tooltip("Offenes Nullspiel.");
            }
            o->end();
          }
           {fltk::Button* o = hand = new fltk::Button(75, 190, 165, 25, "Skat aufnehmen");
            o->labelfont(fltk::HELVETICA_BOLD);
            o->callback((fltk::Callback*)cb_hand);
            o->tooltip("Kein Handspiel.");
          }
           {fltk::Group* o = extra = new fltk::Group(45, 235, 225, 100);
            o->box(fltk::DOWN_BOX);
            o->callback((fltk::Callback*)cb_extra);
            o->when(fltk::WHEN_NEVER);
            o->tooltip("Handspiel");
            o->begin();
             {fltk::CheckButton* o = schneider = new fltk::CheckButton(65, 10, 125, 25, "Schneider");
              o->labelfont(fltk::HELVETICA_BOLD);
              o->callback((fltk::Callback*)cb_schneider);
              o->tooltip("Handspiel, Schneider angesagt.");
            }
             {fltk::CheckButton* o = schwarz = new fltk::CheckButton(65, 40, 110, 25, "Schwarz");
              o->labelfont(fltk::HELVETICA_BOLD);
              o->callback((fltk::Callback*)cb_schwarz);
              o->tooltip("Handspiel, Schwarz angesagt.");
            }
             {fltk::CheckButton* o = ouvert = new fltk::CheckButton(65, 70, 105, 25, "Ouvert");
              o->labelfont(fltk::HELVETICA_BOLD);
              o->callback((fltk::Callback*)cb_ouvert);
              o->tooltip("Offenes Handspiel.");
            }
            o->end();
            extra->user_data(0);
          }
           {fltk::Button* o = play = new fltk::Button(75, 355, 165, 25, "Spiel ansagen");
            o->labelfont(fltk::HELVETICA_BOLD);
            o->callback((fltk::Callback*)cb_play);
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
            o->tooltip("Feld leer lassen um die Skat-Konferenz als Server zu starten.");
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
           {fltk::Button* o = new fltk::Button(165, 190, 195, 25, "Start");
            o->callback((fltk::Callback*)cb_Start);
            o->tooltip("Startet die Skat-Konferenz als Server oder Client.");
          }
           {fltk::Button* o = new fltk::Button(165, 240, 195, 25, "Stats");
            o->callback((fltk::Callback*)cb_Stats);
            o->tooltip("Gibt ein paar Statistiken \303\274""ber die verbundenen Peers im Log-Fenster \
aus.");
          }
           {fltk::CheckButton* o = autostart = new fltk::CheckButton(70, 190, 60, 25, "Auto");
            o->callback((fltk::Callback*)cb_autostart);
            o->tooltip("Wenn aktiviert, dann wird das Netzwerk beim n\303\244""chsten Programmstart a\
utomatisch mitgestartet. Nur sinnvoll f\303\274r Server oder statische IP-Adre\
sse.");
            int i;
            prefs.get("autostart", i, 0);
            autostart->value(i != 0);
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
          o->end();
        }
        o->end();
      }
       {fltk::Group* o = new fltk::Group(0, 25, 960, 675, "Log");
        o->hide();
        o->begin();
        log = new fltk::TextDisplay(0, 0, 960, 675);
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
