// Hyperbolic Rogue

// namespaces for complex features (whirlwind, whirlpool, elec, princess, clearing, 
// mirror, hive, heat + livecaves)

// Copyright (C) 2011-2016 Zeno Rogue, see 'hyper.cpp' for details

#include <map>

namespace whirlwind {

  int fzebra3(cell *c) {
    if(euclid) {
      eucoord x, y;
      decodeMaster(c->master, x, y);
      return 1+((((signed short)(y)+int(50000))/3)%3);
      }
    return zebra3(c);
    }

  void switchTreasure(cell *c) {
    c->item = itNone;
    if(safety) return;
    if(hrand(5000) < PT(100 + 2 * (kills[moAirElemental] + kills[moWindCrow]), 200) && notDippingFor(itWindstone)
      && getGhostcount() < 2)
      c->item = itWindstone;
    else if(hrand(5000) < 20*PRIZEMUL)
      placeLocalOrbs(c);
    }

  int cat(cell *c) {
    if(c->land != laWhirlwind) return 0;
    if(c->wall != waNone && c->wall != waChasm && 
      c->wall != waSea && !isAlchAny(c) &&
      c->wall != waMineMine && c->wall != waFire) return 0;
    if(c->item == itKey || c->item == itOrbYendor) return 0;
    if(airdist(c) < 3) return 0;
    if(c->monst == moHexSnake || c->monst == moHexSnakeTail) return 0;
    return fzebra3(c);
    }
  
  cell *where;
  int dfrom[2], dto[2], qdirs;
  
  int gdist(int d, int e) { return dirdiff(d-e, where->type); }
  
  void calcdirs(cell *c) {
    where = c;
    int d = cat(c);
    qdirs = 0;
    if(d == 0) return;
    int qdf = 0, qdt = 0;
    int cats[7];
    for(int i=0; i<c->type; i++) 
      cats[i] = cat(createMov(c,i));
    for(int i=0; i<c->type; i++)
      if(cats[i] == d) {
        bool c1 = (cats[(i+1)%c->type] != d);
        bool c2 = (cats[(i+c->type-1)%c->type] != d);
        if(c1 && !c2) dto[qdt++] = i;
        if(c2 && !c1) dfrom[qdf++] = i;
        }
    qdirs = qdf;
    if(qdirs == 2) {
      int cur = gdist(dfrom[0], dto[0]) + gdist(dfrom[1], dto[1]);
      int alt = gdist(dfrom[0], dto[1]) + gdist(dfrom[1], dto[0]);
      if(alt < cur) swap(dto[0], dto[1]);
      }
    }

  int mindist(int d, int *tab) {
    if(qdirs == 0) return NODIR;
    if(qdirs == 1) return gdist(d, tab[0]);
    return min(gdist(d, tab[0]), gdist(d, tab[1]));
    }

  int winddir(int d) {
    if(d == -1) return 0;
    int mdf = mindist(d, dfrom);
    int mdt = mindist(d, dto);
    // printf("dir = %d mdf = %d mdt = %d\n", d, mdf, mdt);
    if(mdf < mdt) return -1;
    if(mdf > mdt) return 1;
    return 0;
    }
  
  void build(vector<cell*>& whirlline, int d) {
    again: 
    cell *at = whirlline[size(whirlline)-1];
    cell *prev = whirlline[size(whirlline)-2];
    for(int i=0; i<at->type; i++) 
      if(at->mov[i] && (euclid || at->mov[i]->master->alt) && celldistAlt(at->mov[i]) == d && at->mov[i] != prev) {
        whirlline.push_back(at->mov[i]);
        goto again;
        }
    }
  
  void moveAt(cell *c) {
    if(eq(c->aitmp, sval)) return;
    calcdirs(c);
    if(qdirs != 1) return;
    vector<cell*> whirlline;
    whirlline.push_back(c);
    cell *prev = c;
    cell *c2 = c->mov[dfrom[0]];
    while(true) {
      // printf("c = %p dist = %d\n", c2, c2->mpdist);
      if(c == c2) break;
      calcdirs(c2);
      if(qdirs == 0) break;
      cell *cc2 = c2;
      if(qdirs == 1) whirlline.push_back(c2), c2 = c2->mov[dfrom[0]];
      else if(c2->mov[dto[0]] == prev) 
        c2 = c2->mov[dfrom[1]];
      else
        c2 = c2->mov[dfrom[0]];
      prev = cc2;
      }
    int z = size(whirlline);
    // printf("Cycle built from %p, length = %d\n", c, z);
    for(int i=0; i<z; i++) {
      // printf("%d%c", whirlline[i]->mpdist, whirlline[i]->item ? '*' : ' ');
      whirlline[i]->aitmp = sval;
      if(whirlline[i]->mpdist == BARLEV)
        switchTreasure(whirlline[i]);
      }
    for(int i=0; i<z-1; i++) {
      moveItem(whirlline[i], whirlline[i+1], true);
      }
    for(int i=0; i<z; i++) 
      if(whirlline[i] == cwt.c && cwt.c->item)
        collectItem(cwt.c, true);
    }
  
  void move() {
    sval++;
    for(int i=0; i<size(dcal); i++) {
      cell *c = dcal[i];
      moveAt(c);
      }
    // Keys and Orbs of Yendor always move
    using namespace yendor;
    for(int i=0; i<size(yi); i++) {
      moveAt(yi[i].path[0]);
      moveAt(yi[i].path[YDIST-1]);
      }
    }
  
  cell *jumpFromWhereTo(cell *c, bool player) {
    for(int i=0; i<2; i++) {
      calcdirs(c);
      if(qdirs != 1) return NULL;
      cell *c2 = c->mov[dfrom[0]];
      if(!passable(c, c2, P_JUMP1)) return NULL;
      if(player && i == 0 && !passable(c, c2, P_ISPLAYER)) return NULL;
      c = c2;
      }
    calcdirs(c);
    if(qdirs != 1) return NULL;
    return c;
    }

  cell *jumpDestination(cell *c) {
    for(int i=0; i<2; i++) {
      calcdirs(c);
      if(qdirs != 1) return NULL;
      c = c->mov[dto[0]];
      }
    calcdirs(c);
    if(qdirs != 1) return NULL;
    return c;
    }
  }

namespace elec { 

  bool havecharge, havethunder;
  bool afterOrb; // extra charge from the Orb of Lightning

  enum eCharge {
    ecCharged, ecGrounded, ecIsolator, ecConductor
    };
  
  bool conduct(eCharge cf, eCharge ct) {
    if(ct == ecIsolator) return false;
    if(ct == ecConductor) return true;
    return ct != cf;
    }
  
  eCharge getCharge(cell *c) {
    bool ao = afterOrb && c->ligon;
    
    if(c->wall == waCharged) return ecCharged;
    if(c->wall == waSea || c->wall == waGrounded) return ecGrounded;
    if(c->wall == waSandstone || c->wall == waDeadTroll || 
      c->wall == waDeadTroll2 ||
      c->wall == waVinePlant ||
      c->wall == waMetal || isAlchAny(c)) 
        return c->land == laStorms ? ecConductor : ecGrounded; 
    if(c->wall == waBarrier)
      return ecIsolator;
    if(c->wall == waChasm)
      return ecIsolator;

    if(shmup::on ? isPlayerOn(c) : (c == cwt.c || c == stalemate::moveto || (items[itOrbEmpathy] && isFriendly(c)))) {
      if(items[itOrbShield]) return ecIsolator;
      if(ao) return ecIsolator;
      if(!items[itOrbGhost]) return c->land == laStorms ? ecConductor : ecGrounded;
      }
    
    // if(c->monst && stalemate::moveto) printf("%p: isKilled = %d\n", c, stalemate::isKilled(c));

    if(
      (
        c->monst || (shmup::on ? isPlayerOn(c) : c == cwt.c) || 
        c == stalemate::moveto || c == stalemate::pushto)
      && 
      (c == stalemate::pushto || !stalemate::isKilled(c)) 
      && 
      c->monst != moGhost && c->monst != moIvyDead && c->monst != moIvyNext 
      && !(isDragon(c->monst) && !c->hitpoints)
      )
      return c->land == laStorms ? (ao ? ecCharged : ecConductor) : ecGrounded;

    if(c->land != laStorms)
      return ecGrounded;

    if(ao) return ecCharged;

    return ecIsolator;
    }
  
  // To process conductivity, consider the following graph:
  
  // - edges are between conductors and adjacent charged/grounded/conductor cells
  // - all charged cells are connected to one special cell '0'
  // - all grounded cells are connected to one special cell '1'
  // - cells '0' and '1' are connected
  
  // If A and B are in the same biconnected component, then there is a closed circuit,
  // consisting of all other cells in that component. 
  
  // To find biconnected components, we are using the Hopcroft-Tarjan algorithm.
  
  struct chargedata {
    cell *c;
    int otmp;
    int lowlink;
    bool instack;
    bool fire;
    };
  
  vector<chargedata> charges;
  
  vector<pair<int, int> > xstack;
  
  vector<cell*> chargecells;
  
  bool hasdata(cell *c) {
    return c->aitmp >= 0 && c->aitmp < size(charges) && charges[c->aitmp].c == c;
    }
  
  void connect(int from, cell *c) {
    if(hasdata(c)) {
      // seen again: set the lowlink
      if(!charges[c->aitmp].instack) return;
      // printf("edge %d-%d\n", from, c->aitmp);
      if(c->aitmp < charges[from].lowlink)
        charges[from].lowlink = c->aitmp;
      }
    else {
      int id = size(charges);
      charges.push_back(chargedata());
      chargedata& ch(charges[id]);
      ch.c = c; ch.otmp = c->aitmp; ch.lowlink = id; c->aitmp = id;
      ch.instack = true; ch.fire = false;
      // c->landparam = id;
      
      // printf("edge %d-%d [%s]\n", from, id, dnameof(c->wall));
      
      xstack.push_back(make_pair(from, id));
      
      eCharge chh = getCharge(c);
      
      if(chh == ecGrounded) {
        xstack.push_back(make_pair(id, 0));
        ch.lowlink = 0;
        }
      else if(chh == ecCharged) {
        xstack.push_back(make_pair(id, 1));
        if(from != 1) ch.lowlink = 1;
        }
      
      for(int i=0; i<c->type; i++) {
        cell *c2 = c->mov[i];
        if(!c2) continue;
        if(c2->aitmp == from) continue;
        eCharge ct = getCharge(c2);
        if(conduct(chh, ct))
          connect(id, c2);
        }
      
      // printf("lowlink of %d [%s] = %d\n", id, dnameof(c->wall), ch.lowlink);
      if(ch.lowlink < charges[from].lowlink)
        charges[from].lowlink = ch.lowlink;
  
      if(charges[id].lowlink >= from) {
        while(xstack.back().first != from || xstack.back().second != id) {
          // printf("bcc %d,%d\n", xstack.back().first, xstack.back().second);
          xstack.pop_back();
          }
        // printf("bcc %d,%d\n", xstack.back().first, xstack.back().second);
        xstack.pop_back();
        // printf("\n");
        }
      
      ch.instack = false;
      }
    }
  
  void affect(cell *c) {
    c->ligon = true;
    if(c->monst) {
      if(c->monst == moMetalBeast2 && !c->item)
        c->item = itFulgurite;
      killMonster(c);
      }
    if(isPlayerOn(c)) {
      killThePlayerAt(moLightningBolt, c, 0);
      }
    if(c->wall == waSandstone)
      c->wall = waNone, c->item = itFulgurite;
    if(c->wall == waDeadTroll) c->wall = waCavefloor;
    if(c->wall == waDeadTroll2 || isAlchAny(c) || c->wall == waVinePlant)
      c->wall = waNone;
    /* if(c->wall == waCharged)
      c->wall = waMetal; */
    }
  
  void listChargedCells(cell *c, eCharge last = ecConductor) {
    if(eq(c->aitmp, sval)) return;
    eCharge here = getCharge(c);
    /* if(c->cpdist <= 2) {
      printf("monst=%s ", dnameof(c->monst));
      printf("wall=%s ", dnameof(c->wall));
      printf("c=%p here=%d last=%d\n", c, here, last);
      } */
    if(here == ecIsolator) c->aitmp = sval;
    if(!conduct(last, here)) return;
    if(here == ecCharged) chargecells.push_back(c);
    c->aitmp = sval;
    for(int i=0; i<c->type; i++) {
      cell *c2 = c->mov[i];
      if(c2) listChargedCells(c2, here);
      }
    }
  
  void init() {
    chargecells.clear();
    sval++;
    for(int i=0; i<size(dcal); i++) listChargedCells(dcal[i]);
    
    charges.resize(2); 
    charges[0].lowlink = 0; charges[1].lowlink = 1;
    if(!havecharge) return;
    
    xstack.clear();
    
    for(int i=0; i<size(chargecells); i++) 
      connect(1, chargecells[i]);
  
    havethunder = charges[1].lowlink == 0;
    if(havethunder) {
      for(int i=0; i<size(xstack); i++) { 
        int k = xstack[i].first;
        int l = xstack[i].second;
        // printf("connected %d-%d\n", k, l);
        charges[k].fire = true;
        charges[l].fire = true;
        }
      }
    }
  
  void fire() {
    if(havethunder) {
      addMessage(XLAT("There is a flash of thunder!"));
      drawLightning();
      for(int i=2; i<size(charges); i++) if(charges[i].fire)
        affect(charges[i].c);
      }
    }
  
  void cleanup() {
    for(int i=2; i<size(charges); i++) 
      charges[i].c->aitmp = charges[i].otmp;
    charges.resize(0); 
    }
  
  void draw(cell *c, eCharge what) {
    if(c->ligon) return;
    c->ligon = true;
    for(int i=0; i<c->type; i++) {
      cell *c2 = c->mov[i];
      if(!c2) continue;
      eCharge ch = getCharge(c2);
      if(conduct(what, ch))
        draw(c2, ch);
      }
    }
  
  void drawcharges() {
    for(int i=0; i<size(dcal); i++) 
      if(getCharge(dcal[i]) == ecCharged)
        draw(dcal[i], ecCharged);
    }
  
  bool affected(cell *c) {
    if(c->aitmp >= 0 && c->aitmp < size(charges) && charges[c->aitmp].c == c) 
      return charges[c->aitmp].fire;
    return false;
    }
  
  struct builder {
    builder() { init(); }
    ~builder() { cleanup(); }
    };

  void act() {
    int k = tkills();
    if(cwt.c->land == laStorms && !afterOrb) 
      markOrb(itOrbShield), markOrb(itOrbGhost);
    builder b;
    fire();
    if(!afterOrb)
      achievement_count("ELEC", tkills() - k, 0);
    }
  
  // 0 = no close escape, 1 = close escape, 2 = message already shown
  int lightningfast;
  void checklightningfast() {
    if(lightningfast == 1) {
      addMessage(XLAT("Wow! That was close."));
      lightningfast = 2;
      }
    if(lightningfast > 1) return;
    builder b; 
    if(elec::affected(cwt.c)) 
      lightningfast = 1;
    }

  
  };

namespace princess {

#define EPX 39
#define EPY 21

#define OUT_OF_PRISON 200
#define OUT_OF_PALACE 250

  bool generating = false;
  bool challenge = false;
  bool saved = false;
  bool everSaved = false;
  
  bool forceVizier = false;
  bool forceMouse = false;
  bool gotoPrincess = false;

  int saveHP = 0, saveArmedHP = 0;
  
  struct info {
    int id;         // id of this info
    cell *prison;   // where was the Princess locked
    heptagon *alt;  // alt of the prison
    int bestdist;   // best dist achieved
    int bestnear;   // best dist achieved, by the player
    int value;      // number of Rugs at 120
    cell *princess; // where is the Princess currently
    };
    
  vector<info*> infos;
  
  void assign(info *i) {
    if(i->alt) i->alt->emeraldval = i->id;
    }
  
  void newInfo(cell *c) {
    info *i = new info;
    i->prison = c;
    i->princess = c;
    i->alt = c->master->alt;
    i->id = size(infos);
    i->bestdist = 0;
    i->bestnear = OUT_OF_PRISON;
    infos.push_back(i);
    assign(i);
    }
  
  void newFakeInfo(cell *c) {
    info *i = new info;
    i->prison = NULL;
    i->princess = c;
    i->alt = NULL;
    i->id = size(infos);
    i->bestdist = OUT_OF_PALACE;
    i->bestnear = 0;
    infos.push_back(i);
    assign(i);
    }
  
  info *getPrisonInfo(cell *c) {
    if(euclid) return NULL;
    if(c->land != laPalace) return NULL;
    if(!c->master->alt) return NULL;
    int ev = c->master->alt->emeraldval; // NEWYEARFIX
    if(ev < 0 || ev >= size(infos)) return NULL;
    if(infos[ev]->alt != c->master->alt->alt) return NULL;
    return infos[ev];
    }
  
  info *getPrincessInfo(cell *c) {
    for(int i=0; i<size(infos); i++) if(infos[i]->princess == c) {
      while(i) {
        infos[i]->id = i-1; assign(infos[i]);
        infos[i-1]->id = i; assign(infos[i-1]);
        i--;
        }
      return infos[i];
      }
    return NULL;
    }

  int dist(cell *c) {
    if(c->land != laPalace) return OUT_OF_PALACE;
    else if(euclid) return celldistAlt(c);
    else if(!c->master->alt) return OUT_OF_PRISON;
    else return celldistAlt(c);
    }
  
  void clear() {
    for(int i=0; i<size(infos); i++) delete infos[i];
    infos.clear();
    }
 
  bool bringBackAt(cell *c) {
    if(!c) return false;
    if(!passable(c, NULL, 0)) return false;
    c->monst = moPrincessArmed;
    c->stuntime = 0;
    c->hitpoints = palaceHP();
    drawFlash(c);

    info *inf = NULL;
    for(int i=0; i<size(infos); i++) 
      if(infos[i]->princess && infos[i]->bestdist == OUT_OF_PALACE)
        inf = infos[i];

    if(inf) { inf->princess->monst = moNone; inf->princess = c; }
    else newFakeInfo(c);
    return true;
    }

  void bringBack() {
    if(bringBackAt(cwt.c->mov[cwt.spin])) return;
    for(int i=1; i<size(dcal); i++)
      if(bringBackAt(dcal[i])) return;
    }
  
  void setdist(info *i, int newdist) {
    if(newdist < ALTDIST_ERROR && newdist > i->bestdist) {
      i->bestdist = newdist;
//    printf("Improved dist to %d\n", newdist);
      if(newdist == OUT_OF_PALACE) {
        if(!princess::saved)
          achievement_gain("PRINCESS1");
        princess::saved = true;
        princess::everSaved = true;
        items[itSavedPrincess]++;
        }
      if(newdist == OUT_OF_PRISON && princess::challenge) {
        addMessage(XLAT("Congratulations! Your score is %1.", its(i->value)));
        achievement_gain("PRINCESS2");
        if(!cheater) achievement_score(36, i->value);
        showMissionScreen();
        }
      }
    }

  void save(cell *princess) {
    if(euclid) return;
    princess::info *i = princess::getPrincessInfo(princess);
    if(!i || i->bestdist <= 3) princess->monst = moNone;
    else if(i) setdist(i, OUT_OF_PALACE);
    }

  void move(cell *ct, cell *cf) {
    if(euclid) return;
    princess::info *i = princess::getPrincessInfo(cf);
    if(!i) {
      static bool warn = true;
      // note: OK if mapediting or loading
      if(warn) printf("Warning: unknown princess\n"); 
      if(warn && !cheater) 
        addMessage("Warning: unknown princess (that's a bug, please report)");
      warn = false;
      }
    else {
      i->princess = ct;
      setdist(i, dist(ct));
      // printf("newdist = %d (vs %d)\n", newdist, i->bestdist);
      }
    }

  void mouseSqueak(cell *c) {
    eMonster m = c->monst;
    info *i = getPrisonInfo(c);
    int d = dist(c);

    if(!i)
       addMessage(XLAT("%The1 squeaks in a confused way.", m));
    else if(i->bestdist >= 6)
       addMessage(XLAT("%The1 squeaks gratefully!", m));
    else if(!i->princess)
       addMessage(XLAT("%The1 squeaks hopelessly.", m));
    else if(d > 120)
       addMessage(XLAT("%The1 squeaks in despair.", m));
    else if(d > 90)
       addMessage(XLAT("%The1 squeaks sadly.", m));
    else if(d > 60)
       addMessage(XLAT("%The1 squeaks with hope!", m));
    else if(d > 30)
       addMessage(XLAT("%The1 squeaks happily!", m));
    else
       addMessage(XLAT("%The1 squeaks excitedly!", m));
    }
  
  void line(cell *c) {
    int d = (euclid || c->master->alt) ? celldistAlt(c) : 200;
    eMonster m = c->monst;
  
    static int msgid = 0;
  
    retry:
    if(msgid >= 32) msgid = 0;  
    
    if(msgid == 0 && d < 20 && c->land == laPalace) {
      addMessage(XLAT("%The1 kisses you, and begs you to bring %him1 away from here.", m));
      }
    else if(msgid == 1 && d >= 20 && c->land == laPalace) {
      if(m == moPrincess)
        addMessage(XLAT("\"I want my revenge. Stun a guard and leave him for me!\"", m));
      else
        addMessage(XLAT("\"That felt great. Thanks!\"", m));
      }
    else if(msgid  == 2 && d >= 70 && c->land == laPalace) {
      addMessage(XLAT("\"Bring me out of here please!\"", m));
      }
    else if(msgid == 3 && c->land != laPalace) {
      addMessage(XLAT("%The1 kisses you, and thanks you for saving %him1.", m));
      }
    else if(msgid == 4 && c->land != laPalace && m == moPrincess) {
      addMessage(XLAT("\"I have been trained to fight with a Hypersian scimitar, you know?\"", m));
      }
    else if(msgid == 5 && c->land != laPalace) {
      addMessage(XLAT("\"I would love to come to your world with you!\"", m));
      }
    else if(msgid == 6 && c->land != laPalace) {
      addMessage(XLAT("\"Straight lines stay close to each other forever, this is so romantic!\"", m));
      }
    else if(msgid == 7 && c->land != laPalace) {
      addMessage(XLAT("\"Maps... Just like the world, but smaller... how is that even possible?!\"", m));
      }    
    else {
      msgid++; goto retry;
      }
  
    msgid++;
    }

  void playernear(cell *c) {
    info *i = getPrisonInfo(c);
    int d = dist(c);
    // if(i) printf("d=%d bn=%d\n", d, i->bestnear);
    if(i && d < i->bestnear) {
      if(i->bestnear > 100 && d <= 100) {
        i->value = items[itPalace];
        if(princess::challenge) 
          addMessage(XLAT("Hardness frozen at %1.", its(i->value)));
        }
      i->bestnear = d;
      }
    }
    
  }

namespace clearing {

  struct clearingdata {
    cell *root;
    int dist;
    };
  
  bool buggyplant = false;
  
  std::map<heptagon*, clearingdata> bpdata;
  
  int plantdir(cell *c) {
    generateAlts(c->master);
    for(int i=0; i<7; i++)
      generateAlts(c->master->move[i]);
    int d = celldistAlt(c);
    
    if(purehepta) {
      for(int i=0; i<7; i++) {
        cell *c2 = createMov(c, i);
        if(!pseudohept(c2) && celldistAlt(c2) == d-1)
          return i;
        }
      for(int i=0; i<7; i++) {
        cell *c2 = createMov(c, i);
        if(celldistAlt(c2) == d-1)
          return (i+1) % 7;
        }
      }

    for(int i=1; i<6; i+=2) {
      cell *c2 = createMov(c, i);
      if(celldistAlt(c2) == d-1)
        return i;
      }

    int quseful = 0, tuseful = 0;
    for(int i=1; i<6; i+=2) {
      cell *c2 = c->mov[i];
      if(celldistAlt(c2) == d) {
        bool useful = false;
        for(int j=1; j<6; j++) {
          cell *c3 = createMov(c2, j);
          if(celldistAlt(c3) == d-1)
            useful = true;
          }
        if(useful) quseful++, tuseful += i;
        }
      }
    if(quseful == 1) return tuseful;
    if(quseful == 2) {
      int i;
      if(tuseful == 3+5) i = 3;
      if(tuseful == 5+1) i = 5;
      if(tuseful == 1+3) i = 1;
      if((d & 7) < 4) i = (i+2) % 6;
      return i;
      }
    printf("error in plantdir\n");
    return 1;
    }

  vector<cell*> onpath;
  vector<int>   pdir;
  vector<cell*> rpath;
    
  void generate(cell *c) {
    if(buggyplant) return;
    
    if(euclid) {
      if(pseudohept(c)) return;
      c->monst = moMutant;

      eucoord x, y;
      decodeMaster(c->master, x, y);
      int xco = x * 2 + y + 1;
      c->stuntime = (8-xco/2) & 15;
      // 2, 4, 5, 7
      
      if(pseudohept(createMov(c, 0)))
        c->mondir = 1 + hrand(2) * 4;
      else
        c->mondir = 0;
      return;
      }
    
    // cell *oc = c;
    if(!euclid) generateAlts(c->master);
    if(pseudohept(c)) return;
    heptagon *a = euclid ? NULL : c->master->alt->alt;
    clearingdata& bd(bpdata[a]);
    if(!bd.root) { bd.root = c; bd.dist = 8; }
    
    onpath.clear(); pdir.clear(); rpath.clear();
    
    int steps = 0;
    
    int ds;
    
    while(true) {
      if(c == bd.root) {ds = bd.dist; break; }

      // printf("R %4d C %4d\n", celldistAlt(bd.root), celldistAlt(c));
      if(celldistAlt(c) > celldistAlt(bd.root)) {

        if(c->mpdist <= 6) {
          if(c->monst != moMutant) return; // already cut!
          // ... else simply extend it
          ds = c->stuntime; break;
          }

        int d = plantdir(c);
        steps++;
        onpath.push_back(c); pdir.push_back(d);
        // printf("c [%4d] %p -> %p\n", celldistAlt(c), c, c->mov[d]);
        c = c->mov[d];
        }
      else {
        bd.dist--;
        if(bd.dist < -1000) { 
          for(int i=0; i<steps; i++) 
            onpath[i]->item = itBuggy;
          for(int i=0; i<(int) rpath.size(); i++) 
            rpath[i]->item = itBuggy;
          buggyplant = true;
          printf("buggygen\n");
          return;
          }
        rpath.push_back(bd.root);
        // printf("r [%4d] %p -> %p\n", celldistAlt(bd.root), bd.root, bd.root->mov[plantdir(bd.root)]);
        bd.root = bd.root->mov[plantdir(bd.root)];
        }
      }
    
    // printf("steps = %d dist = %d [%d]\n", steps, bd.dist, oc->mpdist);
    
    onpath.push_back(c); pdir.push_back(plantdir(c));
    while(steps >= 0) {
      c = onpath[steps];
      if(steps == 0) {
        c->monst = moMutant;
        c->mondir = pdir[steps];
        if(pdir[steps] != plantdir(c)) {
          printf("pdir i/ plantdir\n");
          exit(1);
          }
        c->stuntime = ds;
        }
      if(c->mpdist <= 7 && c->monst != moMutant)
        break;
      steps--; ds++;
      }
    }
  }

namespace whirlpool {

  bool escaped = false; // escaped the Whirlpool?

  // next == +1 -> next
  // next == -1 -> prev
  cell *get(cell *c, int next) {
    int i = 0;
    if(!euclid && !c->master->alt) return NULL;
    int d = celldistAlt(c);
    int d2;
    while(true) {
      if(i == c->type) return NULL;
      if(c->mov[i] && (d2 = celldistAlt(c->mov[i])) != d)
        break;
      i++;
      }
    if(i == c->type) return NULL;
    if(d>d2) next = -next;
    for(int j=1; j<c->type; j++) {
      cell *c2 = c->mov[(i+42+next*j) % c->type];
      if(celldistAlt(c2) == d) return c2;
      }
    return NULL;
    }
  
  void build(vector<cell*>& whirlline, int d) {
    again: 
    cell *at = whirlline[size(whirlline)-1];
    cell *prev = whirlline[size(whirlline)-2];
    for(int i=0; i<at->type; i++) 
      if(at->mov[i] && (euclid || at->mov[i]->master->alt) && celldistAlt(at->mov[i]) == d && at->mov[i] != prev) {
        whirlline.push_back(at->mov[i]);
        goto again;
        }
    }
  
  void generate(cell *wto) {
    if(wto->wall == waBoat || wto->monst)
      return;
    
    if(hrand(35000) < 40 + items[itWhirlpool] + yendor::hardness())
      wto->monst = moCShark;
    else if(hrand(5000) < 500)
       wto->wall = waBoat;
    
    if(wto->wall == waBoat && (euclid || wto->master->alt)) { 
      int d = celldistAlt(wto);
      if(yendor::on) d -= 200;
      // 250 : hard
      if(hrand(5000) < 60 + 3 * items[itWhirlpool] + yendor::hardness())
        wto->monst = moPirate;
      if(hrand(5000) < 20 && d < -20 && !tactic::on)
        wto->item = itOrbSafety;
      else if(hrand(5000) < 20 && d < -20 && !tactic::on && markOrb(itOrbLuck))
        wto->item = itOrbSafety;
      else if(hrand(5000) < 20*PRIZEMUL && d < -20)
        placePrizeOrb(wto);
      else if(items[itWhirlpool] >= 10 && hrand(5000) < 20 && d < -15)
        wto->item = itOrbWater;
      else if(d<-10 && hrand(5000) < 1000-d)
        wto->item = itWhirlpool;
      }
    }
  
  void whirlMove(cell *wto, cell *wfrom) {
    // monsters don't move
    if(wfrom && (wfrom == cwt.c || wfrom->monst))
      return;
    // disappear
    if(!wto) { wfrom->wall = waSea; wfrom->item = itNone; }
    
    if(wfrom && wto && wfrom->wall == waBoat && wto->wall == waSea && !wto->monst) {
      wfrom->wall = waSea; wto->wall = waBoat;
      }
    
    if(wfrom && wto && wfrom->item && !wto->item && wfrom->wall != waBoat) {
      // Keys and Orbs of Yendor never disappear!
      if(wfrom->item == itKey || wfrom->item == itOrbYendor)
        for(int i=0; i<wto->type; i++) createMov(wto, i);
      moveItem(wfrom, wto, false);
      }
    
    if(wto && !wfrom) 
      generate(wto);
    }
  
  void moveAt(cell *c) {
    if(c->land != laWhirlpool) return;
    if(eq(c->aitmp, sval)) return;
    if(!(euclid || c->master->alt)) return;
    cell *c2 = get(c, 1);
    if(!c2) return;
    int d = celldistAlt(c);
    vector<cell*> whirlline;
    whirlline.push_back(c);
    whirlline.push_back(c2);
    build(whirlline, d);
    reverse(whirlline.begin(), whirlline.end());
    build(whirlline, d);
    int z = size(whirlline);
    
    for(int i=0; i<z; i++) 
      whirlline[i]->aitmp = sval;
  
    whirlMove(NULL, whirlline[0]);
    
    for(int i=0; i<z-1; i++) 
      whirlMove(whirlline[i], whirlline[i+1]);
    
    whirlMove(whirlline[z-1], NULL);
    }
  
  void move() {
    sval++;
    for(int i=0; i<size(dcal); i++) {
      cell *c = dcal[i];
      moveAt(c);
      }
    // Keys and Orbs of Yendor always move
    using namespace yendor;
    for(int i=0; i<size(yi); i++) {
      moveAt(yi[i].path[0]);
      moveAt(yi[i].path[YDIST-1]);
      }
    }
  }

namespace mirror {

  bool cellMirrorable(cell *c) {
    return 
      c->wall == waNone || c->wall == waCavefloor || isAlchAny(c) ||
      c->wall == waFrozenLake || c->wall == waDeadfloor || c->wall == waDeadfloor2 ||
      c->wall == waGiantRug || c->wall == waCIsland || c->wall == waCIsland2 ||
      c->wall == waGargoyleFloor || c->wall == waRubble ||
      c->wall == waGargoyleBridge || c->wall == waTempFloor || c->wall == waTempBridge;
    }
  
  void createMM(cellwalker& cw, eMonster type) {
    if(type == moLightningBolt)
      castLightningBolt(cw);
    else if(cw.c->monst == moNone && cellMirrorable(cw.c) && cw.c != cwt.c)  {
      cw.c->monst = type;
      cw.c->mondir = cw.spin;
      }
    }
  
  void createMirrors(cell *c, int dir, eMonster type) {
    cellwalker C(c, dir);
    
    if(type == moMirror) type = moMirage;
    else if(type == moMirage) type = moMirror;
  
    for(int i=0; i<c->type; i++) {
      cwstep(C);
      if(C.c->type == c->type) {
        cwspin(C, i);
        createMM(C, type);
        cwspin(C, -i);
        }
      cwstep(C);
      cwspin(C, 1);
      }
    }
  
  void createMirages(cell *c, int dir, eMonster type) {
    cellwalker C(c, dir);
    if(purehepta) {
      for(int i=0; i<c->type; i++) {
        cellwalker C2 = C;
        cwstep(C2);
        cwspin(C2, 3);
        cwstep(C2);
        cwspin(C2, 5);
        cwstep(C2);
        cwspin(C2, 3);
        cwspin(C2, -i);
        createMM(C2, type);
        cwspin(C, 1);
        }
      return;
      }
    for(int i=0; i<6; i++) {
      cwstep(C);
      if(C.c->type == 6) {
        cwspin(C, 2);
        cwstep(C);
        cwspin(C, 4-i);
        createMM(C, type);
        cwspin(C, 6-4+i);
        cwstep(C);
        cwspin(C, 2);
        cwstep(C);
        cwspin(C, 2-i);
        createMM(C, type);
        cwspin(C, 6-2+i);
        cwstep(C);
        cwspin(C, 2);
        }
      cwstep(C);
      cwspin(C, 1);
      }
    }
  
  void spin(int d) {
  
    for(int i=0; i<size(mirrors); i++) {
      cell *c = mirrors[i];
      if(c->monst == moMirror) 
        mirrors[i]->mondir = (mirrors[i]->mondir - d + 42) % mirrors[i]->type;
      if(c->monst == moMirage)
        mirrors[i]->mondir = (mirrors[i]->mondir + d + 42) % mirrors[i]->type;
      }
  
    }
  
  void destroy() {
    for(int i=0; i<size(mirrors); i++) {
      cell *c = mirrors[i];
      eMonster m = c->monst;
      if(isMimic(m)) c->monst = moNone;
      }
    mirrors.clear();
    }
  
  void destroyStray() {
    for(int i=0; i<size(mirrors2); i++) {
      cell *c = mirrors2[i];
      if(c->cpdist > 7 && isMimic(c)) {
        c->monst = moNone;
        }
      }
    }
  
  void go(bool fwd) {
    int tk = tkills();
    int nummirage = 0;
    mirrors2.clear();
    for(int i=0; i<size(mirrors); i++) {
      cell *c = mirrors[i];
      eMonster m = c->monst;
      if(isMimic(m)) {
        if(m == moMirage) nummirage++;
        int dir = c->mondir;
        cell *c2 = c->mov[dir];
        if(c2 && !isMimic(c2) && canAttack(c,m,c2,c2->monst, 0))
          killWithMessage(c2, true, m);
        if(c2->wall == waBigTree)
          c2->wall = waSmallTree;
        else if(c2->wall == waSmallTree)
          c2->wall = waNone;
        if(!fwd) continue;
        c->monst = moNone;
        if(!c2) continue;
        if(!passable(c2, c, P_MONSTER | P_MIRROR))  continue;
        if(isWorm(c2)) continue;
        if(c2->monst == moGreater) {
          c2->monst = moLesser; continue;
          }
        if(c2->monst == moGreaterM) {
          c2->monst = moLesserM; continue;
          }
        if(c2 == cwt.c) {
          addMessage(XLAT("You join %the1.", m));
          continue;
          }
        if(isMimic(c2)) {
          addMessage(XLAT("Two of your images crash and disappear!"));
          c2->monst = moNone;
          continue;
          }
        if(isIvy(c2) || c2->monst) {
          // killIvy(c2);
          continue;
          }
        c->monst = m; moveMonster(c2, c);
        empathyMove(c, c2, dir);
        mirrors2.push_back(c2);      
        }
      }
    for(int i=0; i<size(mirrors2); i++) {
      cell *c = mirrors2[i];
      eMonster m = c->monst;
      if(c->wall == waMirror) {
        addMessage(XLAT("%The1 breaks the mirror!", m));
        createMirrors(c, c->mondir, m);
        c->wall = waNone;
        }
      if(c->wall == waCloud) {
        addMessage(XLAT("%The1 disperses the cloud!", m));
        createMirages(c, c->mondir, m);
        c->wall = waNone;
        }
      }
    achievement_count("MIRRORKILL", tkills(), tk);
    achievement_count("MIRAGE", nummirage, 0);
    }

  }

namespace hive {

  struct buginfo_t {
    cell *where;
    short dist[BUGCOLORS];
    };
  
  vector<buginfo_t> buginfo;
  
  vector<int> bugqueue[BUGCOLORS];
  vector<int> bugqueue4[BUGCOLORS];
  
  struct bugtomove_t {
    int dist, moves, index;
    bugtomove_t(int d, int m, int i) { dist=d; moves=m; index=i; }
    };
  
  bool operator < (const bugtomove_t& m1, const bugtomove_t& m2) {
    if(m1.dist != m2.dist) return m1.dist < m2.dist;
    if(m1.moves != m2.moves) return m1.moves < m2.moves;
    return false;
    }
  
  vector<bugtomove_t> bugtomove;
  vector<cell*> deadbug;
  vector<cell*> bugcellq;
  
  int bugcount[BUGCOLORS];
  
  bool isBugEnemy(cell *c, int k) {
    if(c == cwt.c && !invismove) return true;
    if(!c->monst) return false;
    if(c->monst == moBug0+k) return false;
    if(isIvy(c)) return false;
    return true;
    }
  
  // list bugs and targets for each color
  #define BUGINF 29999
  
  void bugQueueInsert(int k, int i, int d) {
    if(buginfo[i].dist[k] > d) {
      if(buginfo[i].dist[k] != BUGINF) {
        printf("%d -> %d\n", buginfo[i].dist[k], d);
        }
      buginfo[i].dist[k] = d;
      bugqueue[k].push_back(i);
      }
    }
  
  void bugcell(cell *c) {
    short& i(c->aitmp);
    if(i >= 0 && i < size(buginfo) && buginfo[i].where == c)
      return;
    i = size(buginfo);
    buginfo.resize(i+1);
    buginfo_t& b(buginfo[i]);
    b.where = c;
    for(int k=0; k<BUGCOLORS; k++) {
      b.dist[k] = BUGINF;
      bool havebug = false, haveother = false;
      for(int dir=0; dir<c->type; dir++) {
        cell *c2 = c->mov[dir];
        if(c2 && isBugEnemy(c2,k) && canAttack(c,eMonster(moBug0+k),c2,c2->monst, AF_TOUGH | AF_NOSHIELD | AF_GETPLAYER)) {
          if(isBug(c2)) havebug = true;
          else haveother = true;
          }
        }
      if(havebug) bugQueueInsert(k, i, 0);
      else if(haveother) bugqueue4[k].push_back(i);
      }
  /*// bugs communicate if the distance is at most 2
    // also all nearby cells are inserted to the buginfo structure
    if(size(buginfo) < 30000) {
      for(int dir=0; dir<c->type; dir++) {
        cell *c2 = c->mov[dir];
        if(c2) {
          // if(isBug(c)) bugcellq.push_back(c2); => does not help...
          for(int t=0; t<c2->type; t++)
            if(c2->mov[t] && isBug(c2->mov[t]))
              bugcellq.push_back(c2), 
              bugcellq.push_back(c2->mov[t]);
          }
        }
      }*/
    
    // use pheromones!
    if(c->land == laHive && c->landparam > 1 && c->wall != waWaxWall) {
      c->landparam --;
      for(int dir=0; dir<c->type; dir++) {
        cell *c2 = c->mov[dir];
        if(c2) {
          for(int t=0; t<c2->type; t++)
            if(c2->mov[t])
              bugcellq.push_back(c2), 
              bugcellq.push_back(c2->mov[t]);
          }
        }
      }
    }
  
  int last_d = -1;
  
  void handleBugQueue(int k, int t) {
    int i = bugqueue[k][t];
    buginfo_t& b(buginfo[i]);
    cell *c = b.where;
    int d = b.dist[k];
    last_d = d;
    int goodmoves = 0;
    for(int dir=0; dir<c->type; dir++) {
      cell *c2 = c->mov[dir];
      if(!c2) continue;
      if(c2->aitmp < 0 || c2->aitmp >= size(buginfo)) continue;
      if(!passable(c, c2, P_MONSTER)) continue;
      int j = c2->aitmp;
      if(buginfo[j].where != c2) continue;
      if(buginfo[j].dist[k] < d) goodmoves++;
      bugQueueInsert(k, j, d+1);
      }
    if(isBug(c) && c->monst == moBug0+k) {
      bugcount[c->monst - moBug0]++;
      bugtomove.push_back(bugtomove_t(d,goodmoves,i));
      }
    }
  
  #include <set>
  
  bool fightspam(cell *c) {
    return c->cpdist >= 7 || 
      isMetalBeast(c->monst) || c->monst == moSkeleton ||
      isIvy(c->monst) || isMutantIvy(c->monst);
    }
  
  void movebugs() {
    buginfo.clear();
    for(int k=0; k<BUGCOLORS; k++) bugqueue[k].clear();
    for(int k=0; k<BUGCOLORS; k++) bugqueue4[k].clear();
    for(int k=0; k<BUGCOLORS; k++) bugcount[k] = 0;
    bugtomove.clear();
    deadbug.clear();
    
    int xdcs = size(dcal); for(int i=0; i<xdcs; i++) bugcell(dcal[i]);
    for(int i=0; i<size(bugcellq); i++) bugcell(bugcellq[i]);
    bugcellq.clear();
    
    // printf("buginfo = %d\n", size(buginfo));
    
    for(int k=0; k<BUGCOLORS; k++) {
      int t = 0;
      last_d = -1;
      int invadist = 4 - (items[itRoyalJelly]+10) / 20;
      if(invadist<0) invadist = 0;
      for(; t<size(bugqueue[k]) && last_d < invadist-1; t++) handleBugQueue(k, t);
      for(int u=0; u<size(bugqueue4[k]); u++)
        bugQueueInsert(k, bugqueue4[k][u], invadist);
      bugqueue4[k].clear();
      for(; t<size(bugqueue[k]); t++) handleBugQueue(k, t);
      }
    
    for(int k=0; k<BUGCOLORS; k++) {
      set<int> check;
      for(int t=0; t<size(bugqueue[k]); t++) {
        if(check.count(bugqueue[k][t])) {
          printf("REPETITION! [%d]\n", t);
          }
        check.insert(bugqueue[k][t]);
        }
      }
    
    random_shuffle(bugtomove.begin(), bugtomove.end());
    sort(bugtomove.begin(), bugtomove.end());
    
    int battlecount = 0;
    for(int t=0; t<size(bugtomove); t++) {
      bugtomove_t& bm(bugtomove[t]);
      int i = bm.index;
        
      buginfo_t& b(buginfo[i]);
      cell *c = b.where;
      if(!isBug(c)) continue;
      if(c->stuntime) continue;
      eMonster m = c->monst;
      int k = (m - moBug0) % BUGCOLORS;
      int gmoves[8], q=0, bqual = -1;
      
      if(againstRose(c, NULL)) bqual = -40;
  
      for(int dir=0; dir<c->type; dir++) {
        cell *c2 = c->mov[dir];
        int qual = -10;
        if(!c2) continue;
        else if(againstRose(c, c2)) qual = -50;
        else if(canAttack(c, m, c2, c2->monst, AF_GETPLAYER))
          qual = c2->monst == moDeadBug ? -60: isBugEnemy(c2,k) ? 2 : -20;
        else if(!passable(c2, c, 0)) 
          qual = passable(c2, c, P_DEADLY) ? -30 : -60;
        else if(c2->aitmp < 0 || c2->aitmp >= size(buginfo)) qual = -15;
        else if(buginfo[c2->aitmp].where != c2) qual = -15;
        else if(buginfo[c2->aitmp].dist[k] < b.dist[k])
          qual = 1;
        else if(buginfo[c2->aitmp].dist[k] == b.dist[k])
          qual = 0;
        // printf("%d->#%d %d: %d\n", i, dir, c2->tmp, qual);
        if(qual > bqual) bqual = qual, q=0;
        if(qual == bqual) gmoves[q++] = dir;
        }
      
      if(!q) { if(c->land == laHive) c->landparam += 3; continue; }
      int d = gmoves[hrand(q)];
      cell *c2 = c->mov[d];
      if(c2->monst) {
        eMonster killed = c2->monst;
        if(isBug(killed)) battlecount++;
        else if(!fightspam(c2))
          addMessage(XLAT("%The1 fights with %the2!", c->monst, c2->monst));
        killOrStunMonster(c2);
        // killMonster(c);
        if(isBug(killed)) {
          c2->monst = moDeadBug, deadbug.push_back(c2);
          bugcount[killed - moBug0]--;
          }
  //    c->monst = moDeadBug, deadbug.push_back(c);
        }
      else {
        moveMonster(c2, c);
        // pheromones!
        if(c->land == laHive && c->landparam < 90) c->landparam += 5;
        if(c2->land == laHive && c2->landparam < 90) c2->landparam += 5;
        // if(isHive(c2->land)) c2->land = eLand(laHive0+k);
  /*      if(c2->item == itRoyalJelly && !isQueen(m)) {
          // advance!
          c2->monst = eMonster(m+BUGCOLORS);
          c2->item = itNone;
          } */
        }
      }
    
    // cleanup
    for(int i=0; i<size(deadbug); i++) deadbug[i]->monst = moNone;
    if(battlecount)
      addMessage(XLAT("The Hyperbugs are fighting!"));
      
    int maxbug = 0;
    for(int k=0; k<BUGCOLORS; k++) if(bugcount[k] > maxbug) maxbug = bugcount[k];
    
    achievement_count("BUG", maxbug, 0);
    }
  
  void bugcitycell(cell *c, int d) {
    short& i = c->aitmp;
    if(i >= 0 && i < size(buginfo) && buginfo[i].where == c)
      return;
    i = size(buginfo);
    buginfo_t b;
    b.where = c;
    b.dist[0] = d;
    buginfo.push_back(b);
    }
  
  void createBugArmy(cell *c) {
    int k = randomHyperbug() - moBug0;
    int minbugs = 50, maxbugs = 50;
    int var = 5 + items[itRoyalJelly];
    if(var>25) var=25;
    // minbugs += 100; maxbugs += 100;
    minbugs -= var; maxbugs += var;
    maxbugs += items[itRoyalJelly];
    int numbugs = minbugs + hrand(maxbugs - minbugs + 1);
    
    /* int i = items[itRoyalJelly];
    int chance = 20 + 25 * i + 9000;
    // i=0: 16%
    // i=10: 73%
    // i=50: 1270 vs 6000
    eMonster m = eMonster(moBug0 + hrand(BUGCOLORS));
    if(c->wall) return;
    for(int i=0; i<c->type; i++) {
      cell *c2 = createMov(c,i);
      if(hrand(100+chance) < chance) {
        if(!c2->wall) c2->monst = m;
        for(int j=2; j<=c2->type-2; j++) {
          int jj = (j+c->spn[i]) % c2->type;
          cell *c3 = createMov(c2, jj);
          if(hrand(6000+chance) < chance && !c3->wall)
            c3->monst = m;
          }
        }
      }
    c->monst = eMonster(m + BUGCOLORS); */
  
    int gdir = -1;
    for(int i=0; i<c->type; i++) {
      if(c->mov[i] && c->mov[i]->mpdist < c->mpdist) gdir = i;
      }
    if(!gdir) return;
    cellwalker bf(c, gdir);
    int radius = 9;
    if(chaosmode) radius = 5;
    for(int i=2; i<radius; i++) {
      if(bf.c->type == 6)
        cwspin(bf, 3);
      else
        cwspin(bf, 3 + hrand(2));
      cwstep(bf);
      }
    cell *citycenter = bf.c;
    buginfo.clear();
    
    
    // mark the area with BFS
    bugcitycell(citycenter, 0);
    for(int i=0; i<size(buginfo); i++) {
      buginfo_t& b(buginfo[i]);
      cell *c = b.where;
      int d = b.dist[0];
      // ERRORS!
      if(c->land != laHive && c->land != laNone) return;
      if(c->bardir != NODIR) return;
      if(c->land == laHive && c->landparam >= 100) return;
      // bfs
      if(d < radius) for(int t=0; t<c->type; t++)
        bugcitycell(createMov(c,t), d+1);
      }
  
    // place everything
    for(int i=0; i<size(buginfo); i++) {
      buginfo_t& b(buginfo[i]);
      cell *c = b.where;
      int d = b.dist[0];
      if(d <= 1 && c->wall == waNone)
        c->item = itRoyalJelly;
      c->bardir = NOBARRIERS;
      if(d == 9 || d == 6 || d == 3)
        c->barleft = eLand(d/3),
        c->barright = eLand(k);
      else
        c->barleft = laNone;
      if(numbugs && c->wall == waNone)
        c->monst = eMonster(moBug0 + k), numbugs--;
      c->land = laHive;
      // prevent barriers
      if(c->mpdist == INFD) c->mpdist = BUGLEV;
      }
    }

  }

inline float& HEAT(cell *c) { return c->LHU.heat; }

namespace heat {
  vector<cell*> vinefires;  
  vector<pair<cell*, int> > rosefires;
  
  double absheat(cell *c) {
    if(c->land == laCocytus) return HEAT(c) -.6;
    if(c->land == laIce) return HEAT(c) -.4;
    return 0;
    }
  
  double celsius(cell *c) { return absheat(c) * 60; }
  
  void processheat(double rate = 1, bool tick = true) {
    if(markOrb(itOrbSpeed)) rate /= 2;
    int oldmelt = kills[0];
    
    vector<cell*> offscreen2;
    
    for(int i=0; i<size(offscreen); i++) {
      cell *c = offscreen[i];
      if(c->cpdist > 7) {
        bool readd = false;
        if(isIcyLand(c)) {
          if(HEAT(c) < .01 && HEAT(c) > -.01)
            HEAT(c) = 0;
          else {
            HEAT(c) *= 1 - rate/10;
            readd = true;
            }
          }
        if(hasTimeout(c)) {
          useup(c);
          if(hasTimeout(c)) readd = true;
          }
        if(readd) offscreen2.push_back(c);
        }
      }
    
    offscreen.clear(); swap(offscreen, offscreen2);
  
    /* if(cwt.c->heat > .5)  cwt.c->heat += .3;
    if(cwt.c->heat > 1.)  cwt.c->heat += .3;
    if(cwt.c->heat > 1.4) cwt.c->heat += .5; */
    
    for(int i=0; i<numplayers(); i++) {
      cell *c = playerpos(i);
      double xrate = (c->land == laCocytus && shmup::on) ? rate/3 : rate;
      if(purehepta) xrate *= 1.7;
      if(isIcyLand(c))
        HEAT(c) += (markOrb(itOrbWinter) ? -1.2 : 1.2) * xrate;
      }
    
    vinefires.clear();
    rosefires.clear();
    
    int dcs = size(dcal);
    for(int i=0; i<dcs; i++) {
      bool readd = false;
      cell *c = dcal[i];
      double xrate = (c->land == laCocytus && shmup::on) ? rate/3 : rate;
      if(purehepta) xrate *= 1.7;
      if(c->cpdist > 8) break;
  
      if(hasTimeout(c)) {
        if(tick) useup(c);
        readd = true;
        }
      
      if(isFire(c) && tick) {
        if(c->wall != waPartialFire) for(int i=0; i<c->type; i++) {
          cell *c2 = c->mov[i];
          if(c2 && c2->wall == waNone && c2->land == laRose && c->wparam >= 10)
            rosefires.push_back(make_pair(c2, c->wparam));
          if(c2 && c2->wall == waFire && c2->land == laRose && c->wparam >= 10 && c2->wparam < c->wparam/2)
            rosefires.push_back(make_pair(c2, c->wparam));
          if(c2 && c2->wall == waVinePlant)
            vinefires.push_back(c2);
          if(c2 && c2->wall == waRose)
            vinefires.push_back(c2);
          if(c2 && c2->wall == waSaloon)
            vinefires.push_back(c2);
          if(c2 && c2->wall == waSmallTree && c2->land != laDryForest)
            vinefires.push_back(c2);
          if(c2 && (c2->wall == waWeakBranch || c2->wall == waCanopy || c2->wall == waTrunk || c2->wall == waSolidBranch))
            vinefires.push_back(c2);
          if(c2 && c2->wall == waBonfireOff) activateActiv(c2, false);
          // both halfvines have to be near fire at once
          if(c2 && cellHalfvine(c2) && c->mov[(i+1)%c->type]->wall == c2->wall)
            vinefires.push_back(c2);
          }
        
        // two semifires are required to spread
        if(c->wall == waPartialFire) for(int i=0; i<c->type; i++) {
          cell *c2 = c->mov[i];
          if(c2 && (c2->wall == waVinePlant)) {
            for(int j=0; j<c2->type; j++) if(c2->mov[j] && c2->mov[j]->wall == waPartialFire &&
              c2->mov[j] != c)
              vinefires.push_back(c2);
            }
          }
        }
      if(isIcyLand(c)) {
        if(c->monst == moRanger) HEAT(c) += 3 * xrate;
        if(c->monst == moDesertman) HEAT(c) += 4 * xrate;
        if(c->monst == moMonkey) HEAT(c) += xrate;
        if(c->wall == waDeadTroll) HEAT(c) -= 2 * xrate;
        if(c->wall == waDeadTroll2) HEAT(c) -= 1.5 * xrate;
        if(c->wall == waBigStatue) HEAT(c) -= .5 * xrate;
        if(c->monst == moLesser || c->monst == moLesserM || c->monst == moGreater || c->monst == moGreaterM)
          HEAT(c) += (c->land == laCocytus ? 1.5 : 10) * xrate;
        if(c->monst == moGreaterShark)
          HEAT(c) += 2 * xrate;
        if(c->monst == moCultist) HEAT(c) += 3 * xrate;
        if(c->monst == moCultistLeader) HEAT(c) += 4 * xrate;
        if(c->monst == moPyroCultist) HEAT(c) += 6 * xrate;
        if(c->monst == moFireFairy) HEAT(c) += 6 * xrate;
        if(c->monst == moFireElemental) HEAT(c) += 8 * xrate;
        if(isDragon(c->monst)) HEAT(c) += 2 * xrate;
        if(c->monst == moGhost) HEAT(c) -= xrate;
        if(c->monst == moWaterElemental) HEAT(c) -= xrate;
        if(isFire(c)) HEAT(c) += 4 * xrate;
        if(isPrincess(c->monst)) HEAT(c) += (markEmpathy(itOrbWinter) ? -1.2 : 1.2) * xrate;
        
        forCellEx(ct, c) if(!isIcyLand(ct) && isFire(ct)) 
          HEAT(c) += xrate*.1;
        
        ld hmod = 0;

        for(int j=0; j<c->type; j++) if(c->mov[j]) {
          if(!isIcyLand(c->mov[j])) {
            // make sure that we can still enter Cocytus,
            // it won't heat up right away even without Orb of Winter or Orb of Speed
            if(c->mov[j] == cwt.c && (c->land == laIce || markOrb(itOrbWinter))) 
              hmod += (markOrb(itOrbWinter) ? -1.2 : 1.2) / 4;
            continue;
            }
          ld hdiff = absheat(c->mov[j]) - absheat(c);
          hdiff /= 10;
          if(shmup::on && (c->land == laCocytus || c->mov[j]->land == laCocytus))
            hdiff /= 3;
          if(c->mov[j]->cpdist <= 7)
            HEAT(c->mov[j]) -= hdiff * rate;
          else
            hdiff = -HEAT(c) / 30;
          hmod += hdiff;
          }
        
        HEAT(c) += hmod * rate;
        if(c->monst == moCrystalSage && HEAT(c) >= SAGEMELT) {
          addMessage(XLAT("%The1 melts away!", c->monst));
          killWithMessage(c, false);
          }
        }
      
      if(readd || HEAT(c)) 
        offscreen.push_back(c);
      }
    
    for(int i=0; i<dcs; i++) {
      cell *c = dcal[i];
      if(c->wall == waIcewall && HEAT(c) > .4) c->wall = waNone, kills[0]++;
      if(c->wall == waFrozenLake && HEAT(c) > (c->land == laCocytus ? .6 : .4)) c->wall = waLake, kills[0]++;
  
      if(c->wall == waLake && HEAT(c) < (c->land == laCocytus ? -.4 : .4) && c->monst != moGreaterShark) {
        c->wall = waFrozenLake;
        if(c->monst == moShark || c->monst == moCShark) {
          addMessage(XLAT("%The1 is frozen!", c->monst));
          killWithMessage(c, false);
          }
        }
      }
  
    if(tick) for(int i=0; i<size(vinefires); i++) {
      cell* c = vinefires[i];
      if(c->wall == waNone && c->land == laRose)
        makeflame(c, 6, false);
      else if(c->wall == waVinePlant || c->wall == waSmallTree || c->wall == waSaloon || c->wall == waRose)
        makeflame(c, 6, false);
      else if(c->wall == waSolidBranch || c->wall == waTrunk || c->wall == waWeakBranch || c->wall == waCanopy)
        makeflame(c, 6, false);
      else if(cellHalfvine(c)) destroyHalfvine(c, waPartialFire, 6);
      }
    
    if(tick) for(int i=0; i<size(rosefires); i++) {
      cell* c = rosefires[i].first;
      int qty = rosefires[i].second;
      qty /= 2;
      if(c->wall == waNone && c->land == laRose)
        makeflame(c, qty, false);
      if(c->wparam < qty) c->wparam = qty;
      }
    
    if(kills[0] != oldmelt) bfs();
    }
  
  void dryforest() {
    int dcs = size(dcal);
    for(int i=0; i<dcs; i++) {
      cell *c = dcal[i];
      if(c->cpdist > 8) break;
      if(c->land != laDryForest) continue;
      
      for(int j=0; j<c->type; j++) if(c->mov[j]) {
        if(isFire(c->mov[j])) c->landparam++;
        }
  
      if(c->landparam >= 10) makeflame(c, 10, false), c->landparam = 0;
      }
  
    for(int i=0; i<dcs; i++) {
      cell *c = dcal[i];
      if(c->cpdist > 8) break;
      if(c->land != laDryForest) continue;
      if((c->wall == waBigTree || c->wall == waSmallTree || isFire(c)) && c->landparam >= 1)
        c->wall = waEternalFire;
      }
    }
  }

bool gardener = false;

bool lifebrought = false; // was Life brought to the Dead Caves?

void livecaves() {
  int dcs = size(dcal);
  
  vector<cell*> bringlife;
  
  for(int i=0; i<dcs; i++) {
    cell *c = dcal[i];
    if(c->cpdist > 8) break;
    
    if(c->wall == waCavefloor || c->wall == waCavewall) {
      c->aitmp = 0;
      if(c->monst == moDarkTroll) c->monst = moTroll;
      if(c->item || c->monst || c->cpdist == 0) continue;
      for(int j=0; j<c->type; j++) if(c->mov[j]) {
        if(c->mov[j]->wall == waDeadfloor) c->aitmp++, bringlife.push_back(c->mov[j]);
        else if(c->mov[j]->wall == waDeadwall || (c->mov[j]->wall == waDeadfloor2 && !c->mov[j]->monst))
          c->aitmp--, bringlife.push_back(c->mov[j]);
        else if(c->mov[j]->wall == waCavefloor) c->aitmp++;
        else if(c->mov[j]->wall == waCavewall) c->aitmp--;
        else if(c->mov[j]->wall == waRubble) c->aitmp--;
        else if(c->mov[j]->wall == waGargoyle) c->aitmp--;
        else if(c->mov[j]->wall == waGargoyleFloor) c->aitmp--;
        else if(c->mov[j]->wall == waGargoyleBridge) c->aitmp--;
        else if(c->mov[j]->wall == waDeadTroll) c->aitmp -= 5;
        else if(c->mov[j]->wall == waDeadTroll2) c->aitmp -= 3;
        else if(c->mov[j]->wall == waVinePlant) c->aitmp--;
        else if(chaosmode && c->mov[j]->land != laCaves && c->mov[j]->land != laEmerald) ;
        else if(c->mov[j]->wall != waBarrier) c->aitmp += 5;

        if(c->mov[j]->cpdist == 0 && markOrb(itOrbDigging)) c->aitmp+=100;
        if(items[itOrbEmpathy] && isFriendly(c->mov[j]) && markEmpathy(itOrbDigging))
          c->aitmp+=100;
        if(c->mov[j]->wall == waThumperOn) c->aitmp+=100;
        if(c->mov[j]->wall == waFire) c->aitmp+=100;
        if(c->mov[j]->wall == waBigStatue) c->aitmp-=100;
        if(c->mov[j]->item) c->aitmp+=2;
        if(c->mov[j]->monst == moZombie) c->aitmp += 10;
        if(c->mov[j]->monst == moGhost) c->aitmp += 10;
        if(c->mov[j]->monst == moTentacleGhost) c->aitmp += 10;
        if(c->mov[j]->monst == moFriendlyGhost) c->aitmp += 10;
        if(c->mov[j]->monst == moGargoyle) c->aitmp--;
        if(isDragon(c->mov[j]->monst)) c->aitmp++;
        if(c->mov[j]->monst == moNecromancer) c->aitmp += 10;
        if(c->mov[j]->monst == moWormtail) c->aitmp++;
        if(c->mov[j]->monst == moTentacletail) c->aitmp-=2;
        if(isIvy(c->mov[j])) c->aitmp--;
        if(isDemon(c->mov[j])) c->aitmp-=3;
        // if(c->mov[j]->monst) c->tmp++;
        // if(c->mov[j]->monst == moTroll) c->tmp -= 3;
        }
      }
    else if(c->land == laLivefjord) {
      c->aitmp = 0;
      if(c->monst == moWaterElemental)
        c->aitmp += 1000;
      if(isPlayerInBoatOn(c) && markOrb(itOrbWater))
        c->aitmp += 1000;
      if(c->monst == moEarthElemental)
        c->aitmp -= 1000;
      if(isPlayerOn(c) && markOrb(itOrbDigging))
        c->aitmp -= 1000;
      for(int j=0; j<c->type; j++) if(c->mov[j]) {
        cell *c2 = c->mov[j];
        if(c2->wall == waNone || c2->wall == waStrandedBoat)
          c->aitmp -= (c2->land == laLivefjord ? 1 : 100);
        if(c2->wall == waTempFloor || c2->wall == waTempBridge)
          ;
        else if(c2->wall == waDeadTroll || c2->wall == waDeadTroll2 || c2->wall == waThumperOn || isFire(c2) || snakelevel(c2))
          c->aitmp -= 10;
        if(c2->wall == waBigStatue)
          c->aitmp -= 10;
        if(c2->wall == waSea || c2->wall == waBoat)
          c->aitmp += (c2->land == laLivefjord ? 1 : 100);
        if(c2->monst == moWaterElemental)
          c->aitmp += 1000;
        if(c2 == cwt.c && c2->wall == waBoat && markOrb(itOrbWater))
          c->aitmp += 1000;
        if(c2->monst == moEarthElemental)
          c->aitmp -= 1000;
        if(c2 == cwt.c && markOrb(itOrbDigging))
          c->aitmp -= 1000;
        if(items[itOrbEmpathy] && isFriendly(c2) && markEmpathy(itOrbDigging))
          c->aitmp -= 1000;

        if(c2->wall == waBarrier) {
          bool landbar = false;
          for(int k=0; k<c2->type; k++)
            if(c2->mov[k]) {
              cell *c3 = c2->mov[k];
              if(!isSealand(c3->land))
                landbar = true;
              }
          if(landbar) c->aitmp -= 5;
          else c->aitmp += 5;
          }
        }
      }
    }

  for(int i=0; i<dcs; i++) {
    cell *c = dcal[i];
    if(c->cpdist > 8) break;
    if(c->wall == waCavefloor || c->wall == waCavewall) {
  //  if(c->land != laCaves) continue;
  //  if(c->wall == waThumper || c->wall == waBonfire) continue;
      
      if(c->aitmp > 0) c->wall = waCavefloor;
      if(c->aitmp < 0) {
        c->wall = waCavewall;
        if(c->land != laCaves && c->land != laDeadCaves && c->land != laEmerald && !gardener) {
          gardener = true;
          achievement_gain("GARDENER");
          }
        }
      }
    else if(c->land == laLivefjord) {
      if(c->aitmp > 0 && c->wall == waStrandedBoat) c->wall = waBoat;
      if(c->aitmp > 0 && c->wall == waNone) {
        if(c->item && c->cpdist == 1 && markOrb(itOrbWater))
          collectItem(c);
        c->wall = waSea;
        }
      if(c->aitmp < 0 && c->wall == waBoat) c->wall = waStrandedBoat;
      if(c->aitmp < 0 && c->wall == waSea) c->wall = waNone;
      }     
    }
  
  for(int i=0; i<size(bringlife); i++) {
    cell *c = bringlife[i];
    if(c->land == laDeadCaves && !lifebrought) { 
      lifebrought = true;
      achievement_gain("LIFEBRINGER");
      }
    if(c->wall == waDeadfloor) c->wall = waCavefloor;
    if(c->wall == waDeadfloor2) c->wall = waCavewall;
    if(c->wall == waDeadwall) c->wall = waCavewall;
    if(c->wall == waCavewall && c->item) c->wall = waCavefloor;
    if(c->land == laDeadCaves) c->land = laCaves;
    if(c->item == itSilver) c->item = itGold;
    if(c->item == itGreenStone) c->item = itOrbLife;
    if(c->monst == moEarthElemental) {
      addMessage(XLAT("%The1 is destroyed by the forces of Life!", c->monst));
      killWithMessage(c, false);
      c->item = itOrbDigging;
      }
    }
  }

/* evolver */

namespace tortoise {
  map<cell*, cell*> emap;
  map<cell*, int> babymap;
  
  enum tflag {
    tfShell, tfScute0, tfScute1, tfScute2, tfScute3,
    tfEdge1, tfEdge, tfEdge3,
    tfLongNeck, tfFront, tfRear, tfTail,
    tfEyeHue, tfShellHue, tfScuteHue, tfSkinHue,
    tfShellSat, tfScuteSat, tfSkinSat,
    tfShellDark, tfSkinDark,
    tfCOUNT
    };

  const int numbits = (int) tfCOUNT;
  const int mask = (1<<numbits)-1;

  cell *get(cell *where) {
    if(emap.count(where)) return emap[where];
    return where;
    }

  int getb(cell *where) { return getBits(get(where)); }
  
  int countBits(int c) {
    int bi = 0;
    for(int i=0; i<numbits; i++) if((c >> i)&1) bi++;
    return bi;
    }
  
  int getBit(int bits, int id) { return (bits >> id) & 1; }

  int getRandomBits() { return hrand(1 << numbits); }
  
  bool seek() { return items[itBabyTortoise] % 5; }
  int seekbits;
  double seekval[numbits];
  double currval[numbits];
  
  void update(double& val, double target, int delta) {
    double d = delta / 300.;
    if(d>1) d = 1;
    if(target>val+d) val += d;
    else if(target<val-d) val -= d;
    else val = target;
    }
  
  void updateVals(int delta) {
    int currbits = getBits(cwt.c);
    for(int i=0; i<numbits; i++)
      update(seekval[i], seek() ? getBit(seekbits, i) : .5, delta);
    for(int i=0; i<numbits; i++)
      update(currval[i], getBit(currbits, i), delta);
    }
  
  double getScent(int bits) {
    double res = 0;
    for(int i=0; i<numbits; i++)
      /* if(getBit(bits, i) != getBit(getBits(cwt.c), i))
        res += (1 - 2*getBit(bits, i)); */
      res += (2* seekval[i] - 1) * (getBit(bits, i) - currval[i]);
    
    // seek curr bit => res
    //    1    1   1 => 0
    //    1    1   0 => -1
    //    1    0   1 => +1
    //    1    0   0 => 0
    //    0    1   1 => 0
    //    0    1   0 => +1
    //    0    0   1 => -1
    //    0    0   0 => 0
    return res;
    }
  
  int diff(int bits) { return countBits(bits ^ tortoise::seekbits); }
  int progress(int bits) { return numbits - diff(bits); }
  
  string measure(int bits) {
    return "(" + its(progress(bits)) + "/" + its(tortoise::numbits) + ")";
    }    
  }

namespace dragon {
 
  int whichturn; // which turn has the target been set on
  cell *target; // actually for all Orb of Control

  void pullback(cell *c) {
    int maxlen = 1000;
    while(maxlen-->0) {
      cell *c2 = c->mov[c->mondir];
      if(c2 == cwt.c) mountmove(c, c->mondir, true);
      c->monst = c2->monst;
      c->hitpoints = c2->hitpoints;
      c->stuntime = 2;
      if(c2->mondir == NODIR) { c->mondir = NODIR; c2->monst = moNone; return; }
      c = c2;
      }
    }
  
  cell *findhead(cell *c) {
    cell *cor = c;
    int maxlen=1000;
    findhead:  
    if(maxlen--<0) return c;
    if(c->monst == moDragonHead) return c;
    for(int i=0; i<c->type; i++)
      if(c->mov[i] && isDragon(c->mov[i]->monst) && c->mov[i]->mondir == c->spn[i]) {
        c = c->mov[i]; goto findhead;
        }
    printf("dragon bug #3 (%p -> %p)\n", cor, c); return c;
    }
  
  int bodypart(cell *c, cell *head) {
    int i = 0, j = 0;
    int maxlen = 1000;
    while(maxlen-->0) {
      if(head == c) i = j;
      j++;
      if(head->mondir == NODIR) break;
      head = head->mov[head->mondir];
      }
    if(i == 0) return 'h';
    if(i == 1) return 'l';
    if(i == j-2) return '2';
    if(i == j-1) return 't';
    if(i == 2) return 'w';
    return 0;
    }
  
  void kill(cell *c) {
    int delay = false;
    kills[moDragonHead]++;
    int penalty = 0;
    int maxlen = 1000;
    while(maxlen-->0) {
      makeflame(c, 5, false);
      c->monst = moNone;
      if(c->wall == waFire) {
        if(delay) delay = false;
        else {
          if(c->land != laDragon) penalty += 3;
          if(penalty) penalty--;
          else {
            c->item = itDragon;
            c->landparam = shmup::on ? shmup::curtime : turncount;
            delay = true;
            }
          }
        }
      if(c->mondir == NODIR) break;
      c = c->mov[c->mondir];
      }
    }
  
  int totalhp(cell *c) {
    int total = 0;
    int maxlen = 1000;
    while(maxlen-->0) {
      if(!isDragon(c->monst)) {
        printf("dragon bug #4\n");
        return total; 
        }
      total += c->hitpoints;
      if(c->mondir == NODIR) return total;
      c = c->mov[c->mondir];
      }
    return total;
    }
  
  #define SWAPBITFIELD(x,y,t) { t bak=x; x=y; y=bak; }
  
  void pullfront(cell *c, cell *until) {
    cell *buffer = c;
    int maxlen = 1000;
    while(maxlen-->0) {
      SWAPBITFIELD(c->monst, buffer->monst, eMonster);
      SWAPBITFIELD(c->hitpoints, buffer->hitpoints, int);
      if(c == cwt.c) cwt.c = buffer;
      else if(buffer == cwt.c) mountmove(c, c->mondir, true);
      c->stuntime = 2;
      if(c == until) {
        while(true) {
          if(c->mondir == NODIR) return;
          c = c->mov[c->mondir];
          c->stuntime = 2;
          }
        }
      if(c->mondir == NODIR) { printf("dragon bug\n"); break; }
      c = c->mov[c->mondir];
      if(!c) { printf("dragon bug #2\n"); break; }
      }
    }
  
  bool move(cell *dt, cell *df) {
    if(df->monst == moDragonHead) {
      dt->mondir = neighborId(dt,df);
//    printf("pull back\n");
      pullback(dt);
      dt->stuntime = 2;
      return true;
      }
    if(df->monst == moDragonTail && df->stuntime == 0) {
      cell *head = findhead(df);
      if(df->mondir == NODIR) {
        df->mondir = neighborId(df,dt);
        dt->mondir = NODIR;
//      printf("pull all: head = %p, df=%p, dt=%p\n", head, df, dt);
        pullfront(head, dt);
        }
      else {
        cell *c2 = df->mov[df->mondir];
        if(!c2) return false;
        int id = neighborId(dt, c2);
        if(id == -1) return false;
        dt->mondir = id;
        df->mondir = neighborId(df, dt);
//      printf("pull front: head = %p, df=%p, dt=%p\n", head, df, dt);
        pullfront(head, dt);
        }
      return true;
      }
    return false;
    }

  }