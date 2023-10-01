#include <SDL.h>
#include <cmath>
#include <fstream>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <random>




const int screenWidth = 800;
const int screenHeight = 600;
int tileSize = 16;
int tileSizeX = 16;
int tileSizeY = 16;
struct Tile {
    bool itsWall;
    SDL_Color color;
};
int mapWidth = 0;
int mapHeight = 0;

std::vector<Tile> loadMapFromFile(const std::string& filename)
{
    std::vector<SDL_Color> wallColors;
    std::vector<Tile> map;
    std::ifstream file(filename);

    wallColors.clear();
    std::vector<SDL_Color> availableColors = {
            {255, 0, 0, 255},  // Rojo
            {0, 255, 0, 255},  // Verde
            {0, 0, 255, 255},  // Azul
            {255, 255, 0, 255},  // Amarillo
            {255, 255, 255, 255},  // Amarillo
            {255, 200, 100, 255},  // Amarillo
    };
    int colorIndex = 0;


    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            for( const char& a: line){

                if(a =='1'){
                    int indexRand = std::rand() % 4;
                    SDL_Color wall = availableColors[indexRand];

                    if(wallColors.empty()){
                        wallColors.push_back( wall);
                    }else{
                        if(wallColors[wallColors.size()-1].r == wall.r &&
                           wallColors[wallColors.size()-1].b == wall.b &&
                           wallColors[wallColors.size()-1].g == wall.g
                                ){
                            indexRand = (indexRand>2)? 0: indexRand++;
                            wall = availableColors[indexRand];
                            wallColors.push_back(wall);
                        }
                        else{
                            wallColors.push_back(wall);
                        }
                    }
                }
                mapWidth = (line.size()>mapWidth)? line.size(): mapWidth;
                map.push_back({a=='1', {0,0,0,0}});
            }
            mapHeight++;
        }
        file.close();
    }

    tileSize = screenWidth *0.5 /mapWidth;
    tileSizeX = screenWidth *0.5 /mapWidth;
    tileSizeY = screenHeight /mapHeight;

    for(Tile& tile: map){
        if(tile.itsWall){
            tile.color = wallColors.back();
            wallColors.pop_back();
        }
    }


    return map;
}



void drawWorld(SDL_Renderer* renderer, const std::vector<Tile>& map, float playerX, float playerY, float playerAngle)
{
    int index = 0;
    // Dibuja el mapa 2D en la mitad izquierda de la pantalla
    for (int y = 0; y < mapHeight; y++)
    {
        for (int x = 0; x < mapWidth; x++)
        {
            if (map[y*mapWidth+x].itsWall)
            {
                SDL_Color wallColor = map[y*mapWidth+x].color;// Paredes en blanco
                index++;
                SDL_SetRenderDrawColor(renderer, wallColor.r, wallColor.g, wallColor.b, wallColor.a);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Espacios vacíos en negro
            }

            SDL_Rect rect = {x * tileSizeX, y * tileSizeY, tileSizeX, tileSizeY};
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    for (int xF = 0; xF < screenWidth / tileSize; xF++)
    {
        int x = xF * tileSize;

        float rayAngle = (playerAngle - M_PI / 4) + (x / static_cast<float>(screenWidth)) * (M_PI / 2);
        float rayX = playerX;
        float rayY = playerY ;
        float rayDirX = std::cos(rayAngle);
        float rayDirY = std::sin(rayAngle);

        // Ajusta la distancia de avance del rayo
        float rayStepX = rayDirX * 0.05;
        float rayStepY = rayDirY * 0.05;

        bool hitWall = false;

        while (true)
        {
            int mapX = static_cast<int>(rayX);
            int mapY = static_cast<int>(rayY);

            if (mapX < 0 || mapX >= mapWidth || mapY < 0 || mapY >= mapHeight || map[mapY*mapWidth+mapX].itsWall)
            {
                // Dibuja el rayo de blanco si no ha chocado con la pared aún
                if (!hitWall)
                {
                    int diferenceX = (playerX<rayX)? -1: 1;
                    int diferenceY = (playerY<rayY)? -1: 1;
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                    SDL_RenderDrawLine(renderer, playerX * tileSizeX, playerY * tileSizeY, rayX * tileSizeX +diferenceX, rayY * tileSizeY +diferenceY);
                }
                hitWall = true;
            }

            // Verificación de colisión en cada paso
            if (hitWall)
                break;

            rayX += rayStepX;
            rayY += rayStepY;
        }
    }

    index = 0;
    // Dibuja las columnas 3D en la mitad derecha de la pantalla
    for (int col = 0; col < screenWidth; col++)
    {
        float rayAngle = playerAngle - static_cast<float>(col - screenWidth / 2) / static_cast<float>(screenWidth / 2) * (M_PI / 4);

        float rayX = playerX;
        float rayY = playerY;
        float rayDirX = std::cos(rayAngle);
        float rayDirY = std::sin(rayAngle);

        float distanceToWall = 0;
        float stepSize = 0.05; // Tamaño del paso del rayo

        while (true)
        {
            int mapX = static_cast<int>(rayX);
            int mapY = static_cast<int>(rayY);

            if (mapX < 0 || mapX >= mapWidth || mapY < 0 || mapY >= mapHeight || map[mapY*mapWidth+mapX].itsWall)
            {
                // Calcula la distancia al muro
                distanceToWall = std::sqrt((rayX - playerX) * (rayX - playerX) + (rayY - playerY) * (rayY - playerY));

                // Calcula la altura de la pared en función de la distancia y corrige la perspectiva
                int wallHeight = static_cast<int>(screenHeight / (distanceToWall * std::cos(playerAngle - rayAngle)));

                // Añade un factor de corrección para suavizar el efecto
                wallHeight = wallHeight * 1.0; // Ajusta este valor según tu preferencia

                // Calcula el color de la columna y aplica suavizado
                SDL_Color wallColor;
                if (map[mapY*mapWidth+mapX].itsWall)
                {
                    wallColor = map[mapY*mapWidth+mapX].color; // Paredes en rojo
                }
                else
                {
                    wallColor = {0, 0, 255, 255}; // Espacio vacío en azul
                }

                // Dibuja la columna en la mitad derecha de la pantalla
                SDL_SetRenderDrawColor(renderer, wallColor.r * 0.5, wallColor.g * 0.5, wallColor.b * 0.5, 255); // Aplica suavizado
                SDL_Rect wallRect = {col/2 + screenWidth/2, (screenHeight - wallHeight) / 4 + screenHeight/4, 1, wallHeight/2};
                SDL_RenderFillRect(renderer, &wallRect);

                break;
            }

            rayX += rayDirX * stepSize;
            rayY += rayDirY * stepSize;
        }
    }
}



int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Raycasting Example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    std::vector<Tile> map = loadMapFromFile("../myWorld.txt");

    bool quit = false;
    float playerX = 2.0;
    float playerY = 2.0;
    float playerAngle = 0.0;
    SDL_Event event;

    while (!quit)
    {
        while (SDL_PollEvent(&event) != 0)
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (event.type == SDL_KEYDOWN){
                switch (event.key.keysym.sym) {
                    case SDLK_w:
                        playerY+= 0.1 * std::sin(playerAngle);
                        playerX+= 0.1 * std::cos(playerAngle);
                        break;
                    case SDLK_s:
                        playerY-= 0.1 * std::sin(playerAngle);
                        playerX-= 0.1 * std::cos(playerAngle);
                        break;
                    case SDLK_a:
                        playerX-= 0.1 * std::sin(playerAngle);
                        playerY+= 0.1 * std::cos(playerAngle);
                        break;
                    case SDLK_d:
                        playerX+= 0.1 * std::sin(playerAngle);
                        playerY-= 0.1 * std::cos(playerAngle);
                        break;
                    case SDLK_LEFT:
                        playerAngle+=M_PI/18;
                        break;
                    case SDLK_RIGHT:
                        playerAngle-=M_PI/18;
                        break;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        drawWorld(renderer, map, playerX, playerY, playerAngle);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
