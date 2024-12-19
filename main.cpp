#include <stdlib.h>
#include "raylib.h"
#include <cmath>
#include <string>
#include <vector>
#include <fstream>

const int screenWidth = 800;
const int screenHeight = 600;

class Hittable {
public:
  float x,y,w,h;
  Hittable() {}
  Hittable(float x, float y, float w, float h) {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
  }
  bool Intersects (Hittable hi) {
    return
    !(hi.x > x + w ||
    hi.x + hi.w < x ||
    hi.y > y + h ||
    hi.y + hi.h < y);
  }
};

class Drawable : public Hittable {
protected:
  Color c;
public:
  Drawable() {}
  Drawable(float x, float y, float w, float h, Color c) : Hittable(x, y, w, h) {
    this->c = c;
  }
  virtual void Draw() {
    DrawRectangle(x, y, w, h, c);
  }
};

class Runner : public Drawable {
protected: 
  float xspeed, yspeed;
public:
  Runner() {}
  Runner(float x, float y, float w, float h, Color c, float xspeed, float yspeed) : Drawable(x, y, w, h, c) {
    this->xspeed = xspeed;
    this->yspeed = yspeed;
  }
  virtual void Update() {
  }
};

class Player : public Runner {
  bool grounded;
  bool alive;
  bool win;
public:
  int BoxCast(int xd, int yd, bool noret);
  Player() {
    grounded = 0;
    alive = 1;
    win = 0;
  }
  Player(float x, float y, float w, float h, Color c, float xspeed, float yspeed) : Runner(x, y, w, h, c, xspeed, yspeed) {
    grounded = 0;
    alive = 1;
    win = 0;
  }
  void Update() override;
  bool isAlive() { return alive; }
  bool isWin() { return win; }
};

class Enemy : public Runner {
  float minx, maxx;
  float miny, maxy;
public:
  Enemy() {}
  Enemy(float x, float y, float w, float h, Color c, float xspeed, float yspeed, float minx, float maxx, float miny, float maxy) : Runner(x, y, w, h, c, xspeed, yspeed) {
    this->minx = minx;
    this->miny = miny;
    this->maxx = maxx;
    this->maxy = maxy;
  }
  void Update() override;
};

class Level {
public:
  Level() {}
  int levelId;
  std::vector<Enemy> enemies;
  std::vector<Hittable> deathzones;
  std::vector<Drawable> winzones;
  std::vector<Drawable> platforms;
  Player player;
  void Apply();
};

class _World {
public:
  _World() {
    levelId = LoadLevelId();
    inMainMenu = 1;
    dt = 1.f / TARGET_FPS;
    exit = 0;
    camera.offset = {screenWidth / 2, screenHeight / 2};
    camera.zoom = 1;
    a = 200;
    yspeed = 200;
  }
   inline int LoadLevelId() {
    std::ifstream in("results.txt");
    int res;
    in >> res;
    in.close();
    return res;
  }
   inline void StoreLevelId() {
    std::ofstream out("results.txt");
    out << levelId;
    out.close();
  }
  const int TARGET_FPS = 60;
  float dt;
  float a;
  float yspeed;
  int levelId;
  bool inMainMenu;
  std::vector<Hittable*> deadly;
  std::vector<Hittable> winzones;
  std::vector<Drawable> entities;
  std::vector<Enemy> enemies;
  std::vector<Level> levels;
  Player player;
  Camera2D camera;
  bool exit;
  inline void UpdateCamera() {
    camera.target = { player.x + player.w / 2, player.y + player.h / 2 };
  }
  inline void Draw();
};

_World World;

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
  void Click() {
    Vector2 mp = GetMousePosition();
    if (start.IsPressed((int)mp.x, (int)mp.y)) {
      World.inMainMenu = 0;
      World.levelId = 0;
    }
    if (cont.IsPressed((int)mp.x, (int)mp.y)) {
      World.levelId = World.LoadLevelId();
      World.inMainMenu = 0;
    }
    if (ext.IsPressed((int)mp.x, (int)mp.y)) {
      World.exit = 1;
    }
  }
};

inline void _World::Draw() {
  levelId %= levels.size();
  if (IsKeyPressed(KEY_F1)) {
    World.inMainMenu = 1;
    World.StoreLevelId();
  }
  BeginDrawing();
    ClearBackground(RAYWHITE);
    if (World.inMainMenu) {
      Menu menu;
      menu.Draw();
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        menu.Click();
      }
      if (!World.inMainMenu) {
        levels[levelId].Apply();
      }
    } else {
      if (!player.isAlive()) {
        levels[levelId].Apply();
      }
      if (player.isWin()) {
        levelId++; levelId %= levels.size();
        levels[levelId].Apply();
      }
      UpdateCamera();
      BeginMode2D(World.camera);
        for (int i = 0; i < entities.size(); i++) {
          entities[i].Draw();
        }
        for (int i = 0; i < enemies.size(); i++) {
          enemies[i].Draw();
        }
        player.Draw();
        player.Update();
        for (int i = 0; i < enemies.size(); i++) {
          enemies[i].Update();
        }
      EndMode2D();
    }
  EndDrawing();
}

int Player::BoxCast(int xd, int yd, bool noret = false) {
  float left = x;
  float right = x+w;
  float top = y;
  float bottom = y+h;
  float w_ = 0.2, h_ = 0.2;
  float dx = 0.5 + xspeed / World.TARGET_FPS;
  float dy = 0.5 + World.yspeed / World.TARGET_FPS;
  if (xd < 0) {
    Hittable box(left-w_, top+dy, w_, h-2*dy);
    for (int i = 0; i < World.entities.size() && !noret; i++) {
      if (World.entities[i].Intersects(box)) {
        return 1;
      }
    }
    for (int i = 0; i < World.deadly.size() && noret; i++) {
      if (World.deadly[i]->Intersects(box)) {
        alive = 0;
      }
    }
    for (int i = 0; i < World.winzones.size() && noret; i++) {
      if (World.winzones[i].Intersects(box)) {
        win = 1;
      }
    }
  }
  if (xd > 0) {
    Hittable box(right, top+dy, w_, h-2*dy);
    for (int i = 0; i < World.entities.size() && !noret; i++) {
      if (World.entities[i].Intersects(box)) {
        return 1;
      }
    }
    for (int i = 0; i < World.deadly.size() && noret; i++) {
      if (World.deadly[i]->Intersects(box)) {
        alive = 0;
      }
    }
    for (int i = 0; i < World.winzones.size() && noret; i++) {
      if (World.winzones[i].Intersects(box)) {
        win = 1;
      }
    }
  }
  if (yd > 0) {
    Hittable box(left+dx, bottom, w-2*dx, h_);
    for (int i = 0; i < World.entities.size() && !noret; i++) {
      if (World.entities[i].Intersects(box)) {
        return 1;
      }
    }
    for (int i = 0; i < World.deadly.size() && noret; i++) {
      if (World.deadly[i]->Intersects(box)) {
        alive = 0;
      }
    }
    for (int i = 0; i < World.winzones.size() && noret; i++) {
      if (World.winzones[i].Intersects(box)) {
        win = 1;
      }
    }
  }
  if (yd < 0) {
    Hittable box(left+dx, top-h_, w-2*dx, h_);
    for (int i = 0; i < World.entities.size() && !noret; i++) {
      if (World.entities[i].Intersects(box)) {
        return 1;
      }
    }
    for (int i = 0; i < World.deadly.size() && noret; i++) {
      if (World.deadly[i]->Intersects(box)) {
        alive = 0;
      }
    }
    for (int i = 0; i < World.winzones.size() && noret; i++) {
      if (World.winzones[i].Intersects(box)) {
        win = 1;
      }
    }
  }
  return 0;
}

void Player::Update() {
  BoxCast(1, 1, 1);
  BoxCast(-1, -1, 1);
    if (grounded && IsKeyDown(KEY_SPACE)) {
      yspeed = -World.yspeed;
    }
    float cxspeed;
    if (IsKeyDown(KEY_A)) {
      cxspeed = -xspeed;
    } else if (IsKeyDown(KEY_D)) {
      cxspeed = xspeed;
    } else {
      cxspeed = 0;
    }
    if (BoxCast(cxspeed, 0)) {
      cxspeed = 0;
    }
    x += cxspeed * World.dt;
    grounded = 0;
    if (yspeed >= 0 && BoxCast(0, 1)) {
      yspeed = 0;
      grounded = 1;
    } else if (yspeed < 0 && BoxCast(0, -1)) {
      yspeed = 0;
    } else {
      yspeed += World.a * World.dt;
    }
    if (std::abs(yspeed) > World.yspeed) yspeed = World.yspeed;
    y += yspeed * World.dt;
  }

  void Enemy::Update() {
    if (minx > x || x > maxx) xspeed *= -1;
    x += xspeed * World.dt;
    if (miny > y || y > maxy) yspeed *= -1;
    y += yspeed * World.dt;
  }


  void Level::Apply() {
    World.levelId = levelId;
    World.deadly.clear();
    World.enemies.clear();
    World.entities.clear();
    World.winzones.clear();
    for (int i = 0; i < deathzones.size(); i++) {
      World.deadly.push_back(&deathzones[i]);
    }
    for (int i = 0; i < platforms.size(); i++) {
      World.entities.push_back(platforms[i]);
    }
    for (int i = 0; i < enemies.size(); i++) {
      World.enemies.push_back(enemies[i]);
    }
    for (int i = 0; i < enemies.size(); i++) {
      World.deadly.push_back(&World.enemies[i]);
    }

    for (int i = 0; i < winzones.size(); i++) {
      World.entities.push_back(winzones[i]);
      World.winzones.push_back(winzones[i]);
    }

    World.player = player;
  }



Level firstLevel() {
  Level level;
  level.levelId = 0;
  Player p(100, 100, 50, 50, RED, 120, 0);
  level.player = p;
  level.platforms.push_back(Drawable(10, 300, 600, 20, BLACK));
  level.platforms.push_back(Drawable(800, 250, 600, 20, BLACK));
  level.deathzones.push_back(Hittable(-400, 400, 2000, 20));
  level.winzones.push_back(Drawable(1300, 150, 50, 100, GREEN));
  return level;
}

Level secondLevel() {
  Level level;
  level.levelId = 1;
  Player p(100, 100, 50, 50, RED, 120, 0);
  level.player = p;
  level.platforms.push_back(Drawable(10, 300, 600, 20, BLACK));
  level.platforms.push_back(Drawable(800, 250, 600, 20, BLACK));
  level.deathzones.push_back(Drawable(-400, 400, 2000, 20, BLACK));
  level.winzones.push_back(Drawable(1300, 150, 50, 100, GREEN));
  level.enemies.push_back(Enemy(600, 220, 40, 40, YELLOW, 120, 0, 500, 700, 0, 0));
  return level;
}

Level thirdLevel() {
  Level level;
  level.levelId = 2;
  Player p(100, 100, 50, 50, RED, 120, 0);
  level.player = p;
  level.platforms.push_back(Drawable(10, 300, 600, 20, BLACK));
  level.platforms.push_back(Drawable(800, 250, 600, 20, BLACK));
  level.deathzones.push_back(Drawable(-400, 400, 2000, 20, BLACK));
  level.winzones.push_back(Drawable(1300, 150, 50, 100, GREEN));
  level.enemies.push_back(Enemy(600, 220, 40, 40, YELLOW, 120, 0, 500, 700, 0, 0));
  level.enemies.push_back(Enemy(900, 220, 40, 40, YELLOW, 0, 120, 0, 0, 100, 500));
  return level;
}

Level fourthLevel() {
  Level level;
  level.levelId = 3;
  Player p(100, 100, 50, 50, RED, 120, 0);
  level.player = p;
  level.platforms.push_back(Drawable(10, 300, 600, 20, BLACK));
  level.platforms.push_back(Drawable(800, 250, 600, 20, BLACK));
  level.platforms.push_back(Drawable(1000, 400, 400, 20, BLACK));
  level.platforms.push_back(Drawable(1400, 400, 20, 220, BLACK));
  level.platforms.push_back(Drawable(1000, 600, 400, 20, BLACK));
  level.deathzones.push_back(Drawable(-1000, 700, 4000, 20, BLACK));
  level.winzones.push_back(Drawable(1300, 500, 50, 100, GREEN));
  level.enemies.push_back(Enemy(600, 220, 40, 40, YELLOW, 120, 0, 500, 700, 0, 0));
  level.enemies.push_back(Enemy(900, 220, 40, 40, YELLOW, 0, 120, 0, 0, 100, 500));
  return level;
}

Level fifthLevel() {
  Level level;
  level.levelId = 4;
  Player p(100, 100, 50, 50, RED, 120, 0);
  level.player = p;
  level.platforms.push_back(Drawable(10, 300, 600, 20, BLACK));
  level.platforms.push_back(Drawable(800, 250, 600, 20, BLACK));
  level.platforms.push_back(Drawable(1530, 170, 600, 20, BLACK));
  level.platforms.push_back(Drawable(1700, 299, 400, 20, BLACK));
  level.deathzones.push_back(Drawable(-400, 400, 3000, 20, BLACK));
  level.winzones.push_back(Drawable(1720, 199, 50, 100, GREEN));
  level.enemies.push_back(Enemy(600, 220, 40, 40, YELLOW, 120, 0, 500, 700, 0, 0));
  level.enemies.push_back(Enemy(900, 220, 40, 40, YELLOW, 0, 120, 0, 0, 100, 500));
  level.enemies.push_back(Enemy(1780, 150, 40, 40, YELLOW, 0, 120, 0, 0, 100, 399));
  return level;
}

int main () {
  SetTraceLogLevel(LOG_NONE);
  InitWindow(screenWidth, screenHeight, "c++lab");
  SetTargetFPS(World.TARGET_FPS);

  World.levels.push_back(firstLevel());
  World.levels.push_back(secondLevel());
  World.levels.push_back(thirdLevel());
  World.levels.push_back(fourthLevel());
  World.levels.push_back(fifthLevel());

  while (!World.exit && !WindowShouldClose()) {
    World.Draw();
  }
  World.StoreLevelId();

  CloseWindow();

  return 0;
}
