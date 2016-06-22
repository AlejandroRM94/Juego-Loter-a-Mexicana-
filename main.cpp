#include <iostream>
#include <vector>

#include "Boton.h"
#include "Tablero.h"

#define VENTANA_ALTO 680
#define VENTANA_ANCHO 800

#include "ServerPruebaInterfaz.h"

using namespace std;

vector<Imagen*> generarCartas( );
SDL_Texture* renderTexto( const string &texto, SDL_Color color, int tamano,
                          SDL_Renderer *renderer ) ;
void renderTexturaEnRect( SDL_Texture *texto, SDL_Renderer *renderer, int x, int y, int ancho,
                          int alto );
int pantallaInicio( SDL_Renderer *renderer );
void pantallaJuego( SDL_Renderer *renderer, int servidorfd );

int main( int argc, char **argv ) {
    SDL_Init( SDL_INIT_VIDEO );
    IMG_Init( IMG_INIT_PNG );
    IMG_Init( IMG_INIT_JPG );
    TTF_Init();

    SDL_Window *window = SDL_CreateWindow( "Loteria Mexicana", SDL_WINDOWPOS_UNDEFINED,
                                           SDL_WINDOWPOS_UNDEFINED, VENTANA_ANCHO, VENTANA_ALTO, 0 );
    SDL_Renderer *renderer = SDL_CreateRenderer( window, -1, 0 );
    SDL_Surface *icon = IMG_Load( "img/icono.PNG" );
    SDL_SetWindowIcon( window, icon );

    int servidorfd = pantallaInicio( renderer );
    if( servidorfd >= 0 ) {
        pantallaJuego( renderer, servidorfd );
    }

    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}

vector<Imagen*> generarCartas( ) {
    vector<Imagen*> cartas( 54 );
    for( int i = 0; i < 54; i++ ) {
        stringstream ss;
        ss << "img/cartas/" << ( i + 1 )  << ".JPG";
        cartas[i] = new Imagen( ( i + 1 ), ss.str() );
    }
    return cartas;
}

void renderTexturaEnRect( SDL_Texture *texto, SDL_Renderer *renderer, int x, int y, int ancho,
                          int alto ) {
    SDL_Rect dst = {x, y, ancho, alto};
    SDL_RenderCopy( renderer, texto, nullptr, &dst );
}

SDL_Texture* renderTexto( const string &texto, SDL_Color color, int tamano,
                          SDL_Renderer *renderer ) {
    TTF_Font *font = TTF_OpenFont( "arial.ttf", tamano );
    SDL_Surface *surf = TTF_RenderText_Blended( font, texto.c_str(), color );
    SDL_Texture *texture = SDL_CreateTextureFromSurface( renderer, surf );
    SDL_FreeSurface( surf );
    TTF_CloseFont( font );
    return texture;
}

int pantallaInicio( SDL_Renderer *renderer ) {
    SDL_Texture *texturaTitulo, *textura, *texturaFondo, *texturaImgTitulo;
    SDL_Event event;
    int servidorfd = -1;
    string dirServidor = "127.0.0.1";
    bool terminado = false;

    texturaTitulo = renderTexto( "Direcci�n del servidor", SDL_Color { 0, 0, 0, 255 }, 48, renderer );
    Imagen* img = new Imagen( -1, "img/presentacion.JPG" );
    Imagen* imgTitulo = new Imagen( -1, "img/titulo.PNG" );
    texturaFondo = SDL_CreateTextureFromSurface( renderer, img->imagenSurface() );
    texturaImgTitulo = SDL_CreateTextureFromSurface( renderer, imgTitulo->imagenSurface() );

    SDL_Rect hint = { 0, 300, 800, 80 };

    while ( !terminado ) {
        SDL_StartTextInput();
        while ( SDL_PollEvent( &event ) ) {
            switch ( event.type ) {
            case SDL_QUIT:
                servidorfd = -1;
                terminado = true;
            case SDL_KEYDOWN:
                if( event.key.keysym.sym == SDLK_BACKSPACE ) {
                    if( !dirServidor.empty() ) {
                        dirServidor.pop_back();
                    }
                } else if( event.key.keysym.sym == SDLK_RETURN ) {
                    terminado = true;
                    servidorfd = 1;
                }
                break;
            case SDL_TEXTINPUT:
                dirServidor.append( event.text.text );
                break;
            case SDL_TEXTEDITING:
                dirServidor.append( event.edit.text );
                break;
            }

            SDL_SetRenderDrawColor( renderer, 255, 255, 255, SDL_ALPHA_OPAQUE );
            SDL_RenderClear( renderer );

            textura = renderTexto( dirServidor.c_str(), SDL_Color { 0, 0, 0, 255 }, 48, renderer );

            renderTexturaEnRect( texturaFondo, renderer, 0, 73, 800, 534 );
            renderTexturaEnRect( texturaImgTitulo, renderer, 0, 10, 800, 96 );
            SDL_RenderFillRect( renderer, &hint );
            renderTexturaEnRect( texturaTitulo, renderer, 268, 300, 264, 40 );
            renderTexturaEnRect( textura, renderer, ( 400 - dirServidor.size() * 6 ), 340,
                                 dirServidor.size() * 12, 40 );
            SDL_RenderPresent( renderer );
        }
        SDL_StopTextInput();
    }

    SDL_DestroyTexture( textura );
    SDL_DestroyTexture( texturaTitulo );
    SDL_DestroyTexture( texturaFondo );
    SDL_DestroyTexture( texturaImgTitulo );
    delete img;
    delete imgTitulo;
    return servidorfd; // RETURN FD
}

void pantallaJuego( SDL_Renderer *renderer, int servidorfd ) {
    SDL_Texture *textura, *texturaCartaLanzada;
    SDL_Event event;
    Boton* btnLoteria;
    Tablero* tablero;
    vector<Imagen*> cartas;
    bool terminado;

    SDL_SetRenderDrawColor( renderer, 255, 255, 255, SDL_ALPHA_OPAQUE  );
    SDL_RenderClear( renderer );

    cartas = generarCartas();
    tablero = new Tablero( renderer );

    SDL_SetRenderDrawColor( renderer, 224, 224, 224, SDL_ALPHA_OPAQUE );
    btnLoteria = new Boton( 528, 550, 200, 50, "  Loteria  ", renderer );

    textura = renderTexto( "Lanzada", SDL_Color { 0, 0, 0, 255 }, 48, renderer );
    renderTexturaEnRect( textura, renderer, 528, 20, 200, 40 );

    SDL_Rect rectCartaLanzada = { 515, 70, 228, 350 };
    SDL_SetRenderDrawColor( renderer, 224, 224, 224, SDL_ALPHA_OPAQUE );

    MiniServidor serv;  // Borrar despues
    do {
        SDL_RenderFillRect( renderer, &rectCartaLanzada );
        tablero->reiniciar( serv.pruebaSeleccionados() );
        serv.iniciarReloj();
        terminado = false;
        while ( !terminado ) {
            SDL_RenderPresent( renderer );
            while ( SDL_PollEvent( &event ) ) {
                switch ( event.type ) {
                case SDL_QUIT:
                    terminado = true;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if( tablero->verficaClic( event.motion.x, event.motion.y ) ) {
                        cout << "Enviando al servidor info" << endl;
                    } else if( btnLoteria->validaClic( event.motion.x, event.motion.y ) ) {
                        cout << "clic en loteria" << endl;
                    }
                    break;
                }
            }
            if( serv.hayQueLeer() ) {
                int id = serv.lanzar();
                texturaCartaLanzada = SDL_CreateTextureFromSurface( renderer, cartas[id - 1]->imagenSurface() );
                tablero->agregarCartaLanzada( id );
                SDL_RenderCopy( renderer, texturaCartaLanzada, nullptr, &rectCartaLanzada );
            }
        }
    } while ( !terminado );

    SDL_DestroyTexture( textura );
    SDL_DestroyTexture( texturaCartaLanzada );
    delete btnLoteria;
    delete tablero;
    for( Imagen* img : cartas ) {
        delete img;
    }
}
