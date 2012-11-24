// generated by Fast Light User Interface Designer (fluid) version 2.1000

#ifndef ui_h
#define ui_h
#include <fltk/Window.h>
#include <map>
#include <string>
#include <boost/function/function0.hpp>
#include <fltk/run.h>
#include <fltk/Preferences.h>
#include <fltk/xpmImage.h>
#include "GlImage.h"
#include "GlTable.h"
#include "BidButton.h"
#include <fltk/TabGroup.h>
#include <fltk/Group.h>
#include <fltk/Button.h>
#include <fltk/InvisibleBox.h>
#include <fltk/CheckButton.h>
#include <fltk/Input.h>
#include <fltk/ValueInput.h>
#include <fltk/TextDisplay.h>

namespace SK  {

namespace images  {
}

class UILock  {
public:
  UILock();
  ~UILock();
};

class UIUnlock  {
public:
  UIUnlock();
  ~UIUnlock();
};

class UserInterface  {
  fltk::Preferences prefs;
public:
  std::map<std::string, boost::function<void(void)> > f;
  UserInterface(void);
private:
  fltk::Window *mainwnd;
  inline void cb_mainwnd_i(fltk::Window*, void*);
  static void cb_mainwnd(fltk::Window*, void*);
public:
        GlTable *table;
private:
        inline void cb_table_i(GlTable*, void*);
        static void cb_table(GlTable*, void*);
public:
        GlImage *leftimage;
        GlImage *rightimage;
        GlImage *midimage;
          BidButton *bid;
private:
          inline void cb_bid_i(BidButton*, void*);
          static void cb_bid(BidButton*, void*);
public:
          fltk::Button *fold;
private:
          inline void cb_fold_i(fltk::Button*, void*);
          static void cb_fold(fltk::Button*, void*);
public:
          fltk::InvisibleBox *info;
          fltk::InvisibleBox *gameinfo;
          fltk::Group *trump;
            fltk::Button *diamonds;
private:
            inline void cb_diamonds_i(fltk::Button*, void*);
            static void cb_diamonds(fltk::Button*, void*);
public:
            fltk::Button *hearts;
private:
            inline void cb_hearts_i(fltk::Button*, void*);
            static void cb_hearts(fltk::Button*, void*);
public:
            fltk::Button *spades;
private:
            inline void cb_spades_i(fltk::Button*, void*);
            static void cb_spades(fltk::Button*, void*);
public:
            fltk::Button *clubs;
private:
            inline void cb_clubs_i(fltk::Button*, void*);
            static void cb_clubs(fltk::Button*, void*);
public:
            fltk::Button *grand;
private:
            inline void cb_grand_i(fltk::Button*, void*);
            static void cb_grand(fltk::Button*, void*);
public:
            fltk::Button *null;
private:
            inline void cb_null_i(fltk::Button*, void*);
            static void cb_null(fltk::Button*, void*);
public:
            fltk::Button *nullouvert;
private:
            inline void cb_nullouvert_i(fltk::Button*, void*);
            static void cb_nullouvert(fltk::Button*, void*);
public:
          fltk::Group *hand;
            fltk::Button *schneider;
private:
            inline void cb_schneider_i(fltk::Button*, void*);
            static void cb_schneider(fltk::Button*, void*);
public:
            fltk::Button *schwarz;
private:
            inline void cb_schwarz_i(fltk::Button*, void*);
            static void cb_schwarz(fltk::Button*, void*);
public:
            fltk::Button *ouvert;
private:
            inline void cb_ouvert_i(fltk::Button*, void*);
            static void cb_ouvert(fltk::Button*, void*);
public:
          fltk::Button *skat;
private:
          inline void cb_skat_i(fltk::Button*, void*);
          static void cb_skat(fltk::Button*, void*);
public:
          fltk::Button *announce;
private:
          inline void cb_announce_i(fltk::Button*, void*);
          static void cb_announce(fltk::Button*, void*);
          inline void cb_Austeilen_i(fltk::Button*, void*);
          static void cb_Austeilen(fltk::Button*, void*);
          inline void cb_Neustart_i(fltk::Button*, void*);
          static void cb_Neustart(fltk::Button*, void*);
          inline void cb_Mikrofon_i(fltk::CheckButton*, void*);
          static void cb_Mikrofon(fltk::CheckButton*, void*);
public:
          fltk::Input *address;
private:
          inline void cb_address_i(fltk::Input*, void*);
          static void cb_address(fltk::Input*, void*);
public:
          fltk::ValueInput *port;
private:
          inline void cb_port_i(fltk::ValueInput*, void*);
          static void cb_port(fltk::ValueInput*, void*);
public:
          fltk::ValueInput *bandwidth;
private:
          inline void cb_bandwidth_i(fltk::ValueInput*, void*);
          static void cb_bandwidth(fltk::ValueInput*, void*);
          inline void cb_Verbinden_i(fltk::Button*, void*);
          static void cb_Verbinden(fltk::Button*, void*);
          inline void cb_Stats_i(fltk::Button*, void*);
          static void cb_Stats(fltk::Button*, void*);
public:
          fltk::CheckButton *autostart;
private:
          inline void cb_autostart_i(fltk::CheckButton*, void*);
          static void cb_autostart(fltk::CheckButton*, void*);
public:
          fltk::Input *name;
private:
          inline void cb_name_i(fltk::Input*, void*);
          static void cb_name(fltk::Input*, void*);
public:
          fltk::Input *secret;
private:
          inline void cb_secret_i(fltk::Input*, void*);
          static void cb_secret(fltk::Input*, void*);
public:
        fltk::TextDisplay *log;
  ~UserInterface(void);
private:
  UserInterface(const UserInterface&);
  void operator=(const UserInterface&);
};
}
#endif
