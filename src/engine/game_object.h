//
// Created by dawid on 10/31/26.
//

#ifndef BSRENDERER_GAME_OBJECT_H
#define BSRENDERER_GAME_OBJECT_H

class game_object {
public:
    virtual ~game_object() = default;

    virtual void awake();
    virtual void update(double delta_time);
};

#endif //BSRENDERER_GAME_OBJECT_H