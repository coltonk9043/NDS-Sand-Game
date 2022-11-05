#ifndef BUTTON_H
#define BUTTON_H

#include <nds.h>

class Button{
    public:
        Button(int x, int y, int width, int height, u16* texture, int palletID);
        bool Clicked(int clickX, int clickY);
        void Draw();
        int x, y, width, height;
    private:
        u16* texture;
        int palletID;
};

#endif
