#include <stdlib.h>
#include "raylib.h"
#include <string>
#include <vector>
#include <fstream>

const int screenWidth = 800, screenHeight = 600;

class Button {
  int x, y, w, h, fontSize;
  std::string text;
public:
  Button() {}
  Button(int x, int y, int w, int h, int fontSize, std::string text) {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    this->fontSize = fontSize;
    this->text = text;
  }
  bool IsPressed(int mx, int my) {
    return x <= mx && mx <= x + w && y <= my && my <= y + h;
  }
  void Draw() {
    DrawRectangleLines(x, y, w, h, BLACK);
    int tw = MeasureText(text.c_str(), fontSize);
    DrawText(text.c_str(), x + w / 2 - tw / 2, y + h / 2 - fontSize / 2, fontSize, BLACK);
  }
};

class Platform {
  int x, y, w, h;
  Color c;
  int Inside(double x_, double y_, double w_, double h_, Vector2 p, bool shout) {
    bool res = x_ < p.x && p.x < x_ + w_ && y_ < p.y && p.y < y_ + h_;
    return res;
  }
public:
  Platform(int x, int y, int w, int h) {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    c = BLACK;
  }
  void Draw() {
    DrawRectangle(x, y, w, h, c);
  }
  int Intersects(double x_, double y_, double w_, double h_, bool shout = false) {
    Vector2 mp1{(float)x+2, (float)y};
    Vector2 mp2{(float)x+2, (float)y+h};
    Vector2 mp3{(float)x+w-2, (float)y+h};
    Vector2 mp4{(float)x+w-2, (float)y};
    Vector2 op1{(float)x_,(float) y_};
    Vector2 op2{(float)x_,(float) y_+h_};
    Vector2 op3{(float)x_+w_, (float)y_+h_};
    Vector2 op4{(float)x_+w_, (float)y_};
    return 
      Inside(x, y, w, h, op1, shout) ||
      Inside(x, y, w, h, op2, shout) ||
      Inside(x, y, w, h, op3, shout) ||
      Inside(x, y, w, h, op4, shout) ||
      Inside(x_, y_, w_, h_, mp1, shout) ||
      Inside(x_, y_, w_, h_, mp2, shout) ||
      Inside(x_, y_, w_, h_, mp3, shout) ||
      Inside(x_, y_, w_, h_, mp4, shout);
  }
  void setColor(Color c) { this->c = c; }
};

class Enemy {
  float x, y;
  int w, h, mxx, mnx, mxy, mny, xspeed, yspeed;
  float dt;
public:
  Enemy(int x, int y, int w, int h, int mxx, int mnx, int mxy, int mny, int xspeed, int yspeed, float dt) {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    this->mxx = mxx;
    this->mnx = mnx;
    this->mxy = mxy;
    this->mny = mny;
    this->xspeed = xspeed;
    this->yspeed = yspeed;
    this->dt = dt;
  }
  void Update() {
    if (xspeed && (x > mxx || x < mnx)) xspeed *= -1;
    if (yspeed && (y > mxy || y < mny)) yspeed *= -1;
    x += dt * xspeed;
    y += dt * yspeed;
  }
  bool Intersects(double x_, double y_, double w_, double h_, bool shout = false) {
    Platform p(x, y, w, h);
    return p.Intersects(x_, y_, w_, h_, shout);
  }
  void Draw() {
    DrawRectangle(x, y, w, h, YELLOW);
  }
};

std::vector<Platform> Platforms;
std::vector<Platform> deathzones;
std::vector<Platform> winzones;
std::vector<Enemy>    enemies;

class Player {
  double x, y;
  double w, h, xspeed, yspeed;
  double cxspeed, cyspeed;
  double dt, a;
  bool grounded, alive, win;
  int BoxCast(int xd, int yd, bool noret = false) {
    double h_ = 0.2;
    double w_ = 2;
    if (xd < 0) {
      for (int i = 0; i < Platforms.size(); i++) {
        if (!noret && Platforms[i].Intersects(x-w_, y, w_, h-2, 1)) {
          return 1;
        }
      }
      for (int i = 0; i < winzones.size(); i++) {
        if (winzones[i].Intersects(x-w_, y, w_, h)) {
          win = 1;
        }
      }
      for (int i = 0; i < enemies.size(); i++) {
        if (enemies[i].Intersects(x-w_, y, w_, h, 1)) {
          alive = 0;
        }
      }
    }
    if (xd > 0) {
      for (int i = 0; i < Platforms.size(); i++) {
        if (!noret && Platforms[i].Intersects(x+w, y, w_, h-2, 1)) {
          return 1;
        }
      }
      for (int i = 0; i < winzones.size(); i++) {
        if (winzones[i].Intersects(x+w, y, w_, h)) {
          win = 1;
        }
      }
      for (int i = 0; i < enemies.size(); i++) {
        if (enemies[i].Intersects(x+w, y, w_, h, 1)) {
          alive = 0;
        }
      }
    }
    if (yd < 0) {
      for (int i = 0; i < Platforms.size(); i++) {
        if (!noret && Platforms[i].Intersects(x+2, y-h_, w-4, h_)) {
          return 1;
        } 
      }
      for (int i = 0; i < winzones.size(); i++) {
        if (winzones[i].Intersects(x, y-h_, w, h_)) {
          win = 1;
        }
      }
      for (int i = 0; i < enemies.size(); i++) {
        if (enemies[i].Intersects(x, y-h_, w, h_)) {
          alive = 0;
        } 
      }
    }
    if (yd > 0) {
      for (int i = 0; i < Platforms.size(); i++) {
        if (!noret && Platforms[i].Intersects(x+2, y + h, w-4, h_)) {
          return 1;
        }
      }
      for (int i = 0; i < enemies.size(); i++) {
        if (enemies[i].Intersects(x, y + h, w, h_)) {
          alive = 0;
        }
      }
      for (int i = 0; i < winzones.size(); i++) {
        if (winzones[i].Intersects(x, y + h, w, h_)) {
          win = 1;
        }
      }
      for (int i = 0; i < deathzones.size(); i++) {
        if (alive && deathzones[i].Intersects(x, y + h, w, h_)) {
          alive = 0;
        }
      }
    }
    return 0;
  }
public:
  Player() {}
  Player(Player &p) {
    this->x = p.x;
    this->y = p.y;
    this->w = p.w;
    this->h = p.h;
    this->xspeed = p.xspeed;
    this->yspeed = p.yspeed;
    this->cxspeed = p.cxspeed;
    this->cyspeed = p.cyspeed;
    this->dt = p.dt;
    this->a = p.a;
    grounded = p.grounded; 
    alive = p.alive;
    win = p.win;
  }
  Player(int x, int y, int w, int h, int xspeed, int yspeed, double dt, double a) {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    this->xspeed = xspeed;
    this->yspeed = yspeed;
    this->cxspeed = 0;
    this->cyspeed = 0;
    this->dt = dt;
    this->a = a;
    grounded = 0;
    alive = 1;
    win = 0;
  }
  void ApplyInput() {
    if (IsKeyDown(KEY_A)) {
      cxspeed = -xspeed;
    } else if (IsKeyDown(KEY_D)) {
      cxspeed = xspeed;
    } else {
      cxspeed = 0;
    }
    if (grounded && IsKeyPressed(KEY_SPACE)) {
      cyspeed = -yspeed;
      grounded = 0;
    }
  }
  void Draw() {
    DrawRectangle(x, y, w, h, RED);
  }
  void Update() {
    BoxCast(1, 1, 1);
    BoxCast(-1, -1, 1);
    if (BoxCast(cxspeed, 0)) {
      cxspeed = 0;
    }
    x += cxspeed * dt;
    if (cyspeed >= 0 && !BoxCast(0, 1)) grounded = 0;
    if (cyspeed >= 0 && BoxCast(0, 1)) {
      cyspeed = 0;
      grounded = 1;
    } else if (cyspeed < 0 && BoxCast(0, -1)) {
      cyspeed = 0;
    } else {
      cyspeed += a * dt;
    }
    y += cyspeed * dt;
  }
  Vector2 CamTarget() {
    return Vector2{x + w / 2, y + h / 2};
  }
  bool Alive() {
    return alive;
  }
  bool Win() {
    return win;
  }
};

class Level {
  Player p;
  std::vector<Platform> platforms;
  std::vector<Platform> deathzones;
  std::vector<Platform> winzones;
  std::vector<Enemy> enemies;
public:
  Level (Player &p, std::vector<Platform> &platforms, std::vector<Platform> &deathzones, std::vector<Platform> &winzones, std::vector<Enemy> &enemies) {
    this->p = p;
    this->platforms = platforms;
    this->deathzones = deathzones;
    this->winzones = winzones;
    this->enemies = enemies;
  }
  void LoadLevel(Player &p, std::vector<Platform> &platforms, std::vector<Platform> &deathzones, std::vector<Platform> &winzones, std::vector<Enemy> &enemies) {
    p = this->p;
    platforms = this->platforms;
    deathzones = this->deathzones;
    winzones = this->winzones;
    enemies = this->enemies;
  }
};

Level firstLevel() {
  Player p(100, 100, 50, 50, 120, 200, 1.f / 60, 200);
  std::vector<Platform> pl, de, wi;
  std::vector<Enemy> en;
  pl.push_back(Platform(10, 300, 600, 20));
  pl.push_back(Platform(800, 250, 600, 20));
  de.push_back(Platform(-400, 400, 2000, 20));
  wi.push_back(Platform(1300, 150, 50, 100));
  for (int i = 0; i < wi.size(); i++) wi[i].setColor(GREEN);
  Level level(p, pl, de, wi, en);
  return level;
}

Level secondLevel() {
  Player p(100, 100, 50, 50, 120, 200, 1.f / 60, 200);
  std::vector<Platform> pl, de, wi;
  std::vector<Enemy> en;
  pl.push_back(Platform(10, 300, 600, 20));
  pl.push_back(Platform(800, 250, 600, 20));
  de.push_back(Platform(-400, 400, 2000, 20));
  wi.push_back(Platform(1300, 150, 50, 100));
  en.push_back(Enemy(600, 220, 40, 40, 700, 500, 0, 0, 120, 0, 1.f / 60));
  for (int i = 0; i < wi.size(); i++) wi[i].setColor(GREEN);
  Level level(p, pl, de, wi, en);
  return level;
}

Level thirdLevel() {
  Player p(100, 100, 50, 50, 120, 200, 1.f / 60, 200);
  std::vector<Platform> pl, de, wi;
  std::vector<Enemy> en;
  pl.push_back(Platform(10, 300, 600, 20));
  pl.push_back(Platform(800, 250, 600, 20));
  de.push_back(Platform(-400, 400, 2000, 20));
  wi.push_back(Platform(1300, 150, 50, 100));
  en.push_back(Enemy(600, 220, 40, 40, 700, 500, 0, 0, 120, 0, 1.f / 60));
  en.push_back(Enemy(900, 220, 40, 40, 0, 0, 500, 100, 0, 120, 1.f / 60));
  for (int i = 0; i < wi.size(); i++) wi[i].setColor(GREEN);
  Level level(p, pl, de, wi, en);
  return level;
}

Level fourthLevel() {
  Player p(100, 100, 50, 50, 120, 200, 1.f / 60, 200);
  std::vector<Platform> pl, de, wi;
  std::vector<Enemy> en;
  pl.push_back(Platform(10, 300, 600, 20));
  pl.push_back(Platform(800, 250, 600, 20));
  pl.push_back(Platform(1000, 400, 400, 20));
  pl.push_back(Platform(1400, 400, 20, 220));
  pl.push_back(Platform(1000, 600, 400, 20));
  de.push_back(Platform(-1000, 700, 4000, 20));
  wi.push_back(Platform(1300, 500, 50, 100));
  en.push_back(Enemy(600, 220, 40, 40, 700, 500, 0, 0, 120, 0, 1.f / 60));
  en.push_back(Enemy(900, 220, 40, 40, 0, 0, 500, 100, 0, 120, 1.f / 60));
  for (int i = 0; i < wi.size(); i++) wi[i].setColor(GREEN);
  Level level(p, pl, de, wi, en);
  return level;
}

Level fifthLevel() {
  Player p(100, 100, 50, 50, 120, 200, 1.f / 60, 200);
  std::vector<Platform> pl, de, wi;
  std::vector<Enemy> en;
  pl.push_back(Platform(10, 300, 600, 20));
  pl.push_back(Platform(800, 250, 600, 20));
  pl.push_back(Platform(1530, 170, 600, 20));
  pl.push_back(Platform(1700, 299, 400, 20));
  de.push_back(Platform(-400, 400, 3000, 20));
  wi.push_back(Platform(1720, 199, 50, 100));
  en.push_back(Enemy(600, 220, 40, 40, 700, 500, 0, 0, 120, 0, 1.f / 60));
  en.push_back(Enemy(900, 220, 40, 40, 0, 0, 500, 100, 0, 120, 1.f / 60));
  en.push_back(Enemy(1780, 150, 40, 40, 0, 0, 399, 100, 0, 120, 1.f / 60));
  for (int i = 0; i < wi.size(); i++) wi[i].setColor(GREEN);
  Level level(p, pl, de, wi, en);
  return level;
}

int LoadCurr() {
  int res;
  std::ifstream in("results.txt");
  in >> res;
  return res;
}

class Menu {
  Button start, cont, ext;
public:
  Menu() {
    int w = 200;
    int h = 50;
    int x = (screenWidth - w) / 2;
    int mid = (screenHeight - h) / 2;
    start = Button(x, mid - 100, w, h, 20, "Start");
    cont = Button(x, mid, w, h, 20, "Continue");
    ext = Button(x, mid + 100, w, h, 20, "Exit");
  }
  void Draw() {
    start.Draw();
    cont.Draw();
    ext.Draw();
  }
  void Click(bool &st, int &current_level, bool &ex) {
    Vector2 mp = GetMousePosition();
    st = start.IsPressed((int)mp.x, (int)mp.y);
    if (cont.IsPressed((int)mp.x, (int)mp.y)) {
      current_level = LoadCurr();
      st = 1;
    }
    if (ext.IsPressed((int)mp.x, (int)mp.y)) {
      ex = 1;
      st = 0;
    }
  }
};

void StoreCurr(int current_level) {
  std::ofstream out("results.txt");
  out << current_level;
  out.close();
}

int main () {
  SetTraceLogLevel(LOG_NONE); 
  InitWindow(screenWidth, screenHeight, "c++lab");
  FILE* f;
  if (!(f = fopen("results.txt", "r"))) {
    StoreCurr(0);
  } else {
    fclose(f);
  }

  SetTargetFPS(60);

  bool exit = 0;

  Button b(10, 10, 100, 100, 20, "text");

  Player p;
  Level levels[5] = { firstLevel(), secondLevel(), thirdLevel(), fourthLevel(), fifthLevel() };

  Camera2D camera = { 0 };
  camera.offset = { screenWidth / 2, screenHeight / 2 };
  camera.rotation = 0;
  camera.zoom = 1;

  Menu m;

  bool start = 0;
  int current_level = LoadCurr();
  const int max_level = 5;

  while (!exit && !WindowShouldClose()) {
    camera.target = p.CamTarget();

    if (IsKeyPressed(KEY_F1)) {
      start = 0;
      StoreCurr(current_level);
    }

    BeginDrawing();

      ClearBackground(RAYWHITE);
      if (!start) {
        m.Draw();
        if (IsMouseButtonPressed(0 /* MOUSE_BUTTON_LEFT */)) {
          m.Click(start, current_level, exit);
          if (start) { 
            levels[current_level].LoadLevel(p, Platforms, deathzones, winzones, enemies);
          }
        }
      } else {
      if (!p.Alive()) { levels[current_level].LoadLevel(p, Platforms, deathzones, winzones, enemies); }
      if (p.Win()) { current_level++; current_level %= max_level; levels[current_level].LoadLevel(p, Platforms, deathzones, winzones, enemies); }
      BeginMode2D(camera);
        p.ApplyInput();
        p.Draw();
        for (int i = 0; i < Platforms.size(); i++) Platforms[i].Draw();
        for (int i = 0; i < winzones.size(); i++) winzones[i].Draw();
        for (int i = 0; i < enemies.size(); i++) enemies[i].Draw();
        p.Update();
        for (int i = 0; i < enemies.size(); i++) enemies[i].Update();
      EndMode2D();
      }

    EndDrawing();    
  }
  StoreCurr(current_level);
  CloseWindow();

  return 0;
}
