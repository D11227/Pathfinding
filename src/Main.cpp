#include <SDL2/SDL.h>

#include <iostream>

#include <vector>

#include <array>

#include <algorithm>

#include <math.h>

#include "Globals.h"

#include "Node.h"

#include "Grid.h"

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;

int
        frameCount,
        timerFPS,
        lastFrame,
        fps,
        lastTime;

bool
        running = true,
        isMouseDown = false,
        isMouseUp = false;

Grid grid;

Node start, target;

std::vector<Node> open, closed;
std::vector<Node> path;

void init() {
        grid = Grid(renderer, 10);
        start = Node(renderer, 0, 0, false);
        target = Node(renderer, 9, 9, false);

        open.push_back(start);
}

float heuristic(Node start, Node end) {
        return std::hypot(end.x - start.x, end.y - start.y);
}

void update() {
        if (open.empty()) {
                return;
        }

        Node current = open[0];

        for (std::size_t i = 1; i < open.size(); ++i)
                if (open[i].f < current.f)
                        current = open[i];

        if (current == target) {
                current = grid.getNode(target.x, target.y);

                while (current != start) {
                        path.push_back(current);
                        current = grid.getNode(current.parentX, current.parentY);
                }

                std::reverse(path.begin(), path.end());
                return;
        }

        open.erase(std::remove(open.begin(), open.end(), current), open.end());
        closed.push_back(current);

        for (Node neighbor : grid.getNeighbors(current)) {
                if (neighbor.isWall || (std::find(closed.begin(), closed.end(), neighbor) != closed.end()))
                        continue;

                int newCost = current.g + heuristic(current, neighbor);
                if (newCost < neighbor.g || (std::find(open.begin(), open.end(), neighbor) == open.end())) {
                        neighbor.g = newCost;
                        neighbor.h = heuristic(neighbor, target);
                        neighbor.f = neighbor.g + neighbor.h;

                        grid.setParent(neighbor, current);

                        if (std::find(open.begin(), open.end(), neighbor) == open.end())
                                open.push_back(neighbor);
                }
        }
}

void input() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
                switch (event.type) {
                        case SDL_QUIT: running = false; break;
                }
        }
}

void tick() {
        lastFrame = SDL_GetTicks();
        if (lastFrame >= lastTime + 1000) {
                lastTime = lastFrame;
                fps = frameCount;
                frameCount = 0;
        }
}

void draw() {
        for (auto &node : closed)
                node.draw(255, 106, 73);    // Red.

        for (auto &node : open)
                node.draw(36, 221, 96);      // Green.

        for (auto &node : path)
                node.draw(85, 176, 254);      // Pink.

        start.draw(63, 119, 255);
        target.draw(255, 34, 10);

        grid.draw();
}

void render() {
        SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
        SDL_RenderClear(renderer);

        ++frameCount;
        timerFPS = SDL_GetTicks() - lastFrame;
        if (timerFPS < (1000 / 60)) {
                SDL_Delay((1000 / 60) - timerFPS);
        }

        draw();

        SDL_RenderPresent(renderer);
}

int main(int argc, char const *argv[]) {
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
                std::cout << "SDL_Init() failed" << '\n';
        if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer) < 0)
                std::cout << "SDL_CreateWindowAndRenderer() failed" << '\n';
        SDL_SetWindowTitle(window, "Path Finding");

        init();

        while (running) {
                tick();
                update();
                input();
                render();
        };

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        return 0;
}
