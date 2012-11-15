// generated by Fast Light User Interface Designer (fluid) version 2.1000

#include "ui.h"
//Copyright (C) 2012 Carsten Paproth
using namespace SK;

inline void UserInterface::cb_mainwnd_i(fltk::Window*, void*) {
  mainwnd->hide();
  fltk::unlock();
}
void UserInterface::cb_mainwnd(fltk::Window* o, void* v) {
  ((UserInterface*)(o->user_data()))->cb_mainwnd_i(o,v);
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

UILock::UILock() {
  fltk::lock();
}

UILock::~UILock() {
  fltk::unlock();
}

UserInterface::UserInterface(void):prefs(fltk::Preferences::USER, "cpaproth", "sk") {
  fltk::Window* w;
  fltk::lock();
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
           {fltk::Button* o = new fltk::Button(35, 40, 150, 25, "Neustart Audio Stream");
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
            o->maximum(1e+009);
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
