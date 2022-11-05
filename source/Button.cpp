#include "Button.h"

Button::Button(int x, int y, int width, int height, u16* texture, int palletID){
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->texture = texture;
    this->palletID = palletID;
}

bool Button::Clicked(int clickX, int clickY){
    if(clickX > x && clickX < x+width){
        if(clickY > y && clickY < y+height){
            return true;
        }
    }
    return false;
}

void Button::Draw(){
    oamSet(&oamSub, this->palletID, x, y, 0, this->palletID, SpriteSize_32x32, SpriteColorFormat_16Color, this->texture, 0, false, false, false, false, false); 
}