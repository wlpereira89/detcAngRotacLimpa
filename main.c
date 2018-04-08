#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pdi.h"
#define PI 3.14159265
#define IMG_INICIAL "img/b3.bmp"

Imagem* rotaciona(Imagem* in, float angulo);
float Hough (Imagem* bordas);
Imagem* autocrop (Imagem* in, int corte);

int main ()
{


    Imagem* img = abreImagem (IMG_INICIAL, 3); // imagem com chroma key
    Imagem* cinza = criaImagem (img->largura, img->altura, 1);
    Imagem* binarizada = criaImagem (img->largura, img->altura, 1);
    Imagem* angularizada;
    Imagem* mag = criaImagem (img->largura, img->altura, 1);
    Imagem* ori = criaImagem (img->largura, img->altura, 1);
    Imagem* bordabin = criaImagem (img->largura, img->altura, 1);
    RGBParaCinza(img, cinza);
    salvaImagem(cinza, "resultado/1_EscalaDeCinza.bmp");
    computaGradientes(cinza, 5, NULL, NULL, mag, ori);
    binariza(mag, bordabin, thresholdOtsu(mag));
    salvaImagem(mag, "resultado/bla_Mag.bmp");
    salvaImagem(ori, "resultado/bla_ori.bmp");
    salvaImagem(bordabin, "resultado/bla_b bin.bmp");
    float ang = Hough(bordabin);
    float threshold =  thresholdOtsu(cinza);
    binariza(cinza, binarizada, threshold);
    salvaImagem(binarizada, "resultado/2_Binarizada.bmp");
    angularizada = rotaciona(binarizada, ang);

    salvaImagem(angularizada, "resultado/3_Angularizada.bmp");
    angularizada = autocrop(angularizada, 15);
    salvaImagem(angularizada, "resultado/4_Final.bmp");
    return 0;
}

Imagem* rotaciona (Imagem* in, float ang)
{
    /**
    x = x1 cos teta - y1 sen teta
    y = x1 sen teta + y1 cos teta'

    percorre a img

    **/
    float angulo = ang * PI/180;
    Imagem* out = criaImagem(in->largura*2, in->altura*2,in->n_canais);
    int channel, row, col, y, x;
    int tx = in->largura/2;
    int ty = in->altura/2;
    for (channel = 0; channel < out->n_canais; channel++)
        for (row = 0; row < out->altura; row++)
            for (col = 0; col < out->largura; col++)
                out->dados [channel][row][col]=1;
    for (channel = 0; channel < in->n_canais; channel++)
        for (row = 10; row < in->altura-10; row++)
            for (col = 10; col < in->largura-10; col++)
            {
                y = col*(sin(angulo)) + row*(cos(angulo)) + ty*(1-cos(angulo))+ tx*(sin(angulo));
                y = y + in->altura/2;
                //y' = x*cos o + y*sin o + Ty(1-cos o) + Tx*sin o

                x = col*cos(angulo) - row*sin(angulo) + tx*(1-cos(angulo))+ ty*(sin(angulo));
                x = x + in->largura/2;
                // x' = x*cos o - y*sin o + Tx(1-cos o) + Ty*sin o

                if(y < out->altura && x < out->largura && in->dados [channel][row][col] == 0 && in->largura-1+y>=0 && out->largura-1+x >= 0)
                {
                    if (x>=0 && y>=0)
                        out->dados [channel][y][x] = in->dados [channel][row][col];
                    else
                    {
                       // printf("sai em, x %d para %d e y %d para %d\n", x, col, y, row);
                    }
                }
            }
    return out;
}
float Hough (Imagem* bordas)
{
    int theta;
    float passo = 1;
    float rad = PI/180;

    int channel, row, col, r;
    int largura = sqrt(pow(bordas->altura, 2)+pow(bordas->largura, 2));
    largura++;
    Imagem* hist2d = criaImagem (largura, 180/passo+1, bordas->n_canais);
    for (channel = 0; channel < hist2d->n_canais; channel++)
    {
        for (row = 0; row < hist2d->altura; row++)
        {
            for (col = 0; col < hist2d->largura; col++)
            {
                hist2d->dados [channel][row][col]=0;
            }
        }
    }

    for (channel = 0; channel <bordas->n_canais; channel++)
    {
        for (row = 0; row < bordas->altura; row++)
        {
            for (col = 0; col < bordas->largura; col++)
            {
                if(bordas->dados [channel][row][col] > 0.5) //se for pixel de borda
                {
                    for (theta= 0; theta<180; theta=theta+passo)
                    {
                        r = col*cos(theta*rad) + row*sin(theta*rad);
                        if (r < 0) r *= -1;
                        hist2d->dados[channel][theta][r]+=1;
                    }
                }
            }
        }
    }
    normaliza(hist2d, hist2d, 0, 1);
    salvaImagem(bordas, "resultado/a.bmp");
    salvaImagem(hist2d, "resultado/histograma.bmp");

    int cont;
    int contmaior=0;
    float ang=0;
    for (channel = 0; channel <hist2d->n_canais; channel++)
    {
        for (row = 60; row <= 120; row++)
        {


            cont=0;
            for (col = 0; col < hist2d->largura; col++)
            {
                if (hist2d->dados[channel][row][col]>0.7)
                {
                    cont=cont+1;
                }
            }

            if(cont>contmaior)
            {
                contmaior=cont;
                ang=row*passo;

            }
        }
    }
    if (contmaior==0)
        ang = 90;
    printf("angulo: %.2f, %d app\n", (90-ang), contmaior);


    return (90-ang);
}

Imagem* autocrop (Imagem* in, int corte)
{
    int inicialx = 0;
    int inicialy = 0;
    int finalx, finaly;
    int row, col, channel;
    int flag = 0;

    for (channel = 0; channel < in->n_canais; channel++)
    {
        for (row = 0; row < in->altura && flag == 0; row++)
        {
            inicialy=row;
            for (col = 0; col < in->largura && flag == 0; col++)
            {
                if (in->dados [channel][row][col]!=1)
                    flag = 1;
            }
        }
    }

    flag = 0;
    for (channel = 0; channel < in->n_canais; channel++)
    {
        for (col = 0; col < in->largura && flag == 0; col++)
        {
            inicialx=col;
            for (row = 0; row < in->altura && flag == 0; row++)
            {
                if (in->dados [channel][row][col]!=1)
                    flag = 1;
            }
        }
    }
    flag = 0;
    for (channel = 0; channel < in->n_canais; channel++)
    {
        for (row = in->altura-1; row >= 0  && flag == 0; row--)
        {
            finaly=row;
            for (col = in->largura-1; col >= 0  && flag == 0; col--)
            {
                if (in->dados [channel][row][col]!=1)
                    flag = 1;
            }
        }
    }
    flag = 0;
    for (channel = 0; channel < in->n_canais; channel++)
    {
        for (col = in->largura-1; col >= 0 && flag == 0; col--)
        {
            finalx=col;
            for (row = in->altura-1; row >= 0  && flag == 0; row--)
            {
                if (in->dados [channel][row][col]!=1)
                    flag = 1;
            }
        }
    }



    if (inicialx>corte)
        inicialx = inicialx-corte;
    else
        inicialx = 0;

    if (inicialy>corte)
        inicialy = inicialy-corte;
    else
        inicialy = 0;

    if (finalx+corte<in->largura)
        finalx = finalx+corte;
    else
        finalx = in->largura;

    if (finaly+corte<in->altura)
        finaly = finaly+corte;
    else
        finaly = in->altura;

    Imagem* out = criaImagem(finalx-inicialx, finaly-inicialy, in->n_canais);
    for (channel = 0; channel < in->n_canais; channel++)
    {
        for (col = 0; col < out->largura; col++)
        {
            for (row = 0; row < out->altura; row++)
            {
                out->dados [channel][row][col]=in->dados [channel][row+inicialy][col+inicialx];
            }
        }
    }
    return out;
}
