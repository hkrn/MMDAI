#ifndef TILEDSTAGE_H
#define TILEDSTAGE_H

#include <QtCore/QString>
#include <QtGui/QMatrix4x4>
#include <LinearMath/btVector3.h>

class btRigidBody;
class Delegate;
class TiledStageInternal;
class World;

class TiledStage
{
public:
    TiledStage(Delegate *delegate, World *world);
    ~TiledStage();

    void loadFloor(const QString &path);
    void loadBackground(const QString &path);
    void render();
    void setSize(float width, float height, float depth);
    void updateShadowMatrix(const btVector3 &position);

private:
    void buildFloor(float width, float height);
    void destroyFloor();

    TiledStageInternal *m_floor;
    TiledStageInternal *m_background;
    btRigidBody *m_floorRigidBody;
    Delegate *m_delegate;
    World *m_world;
    QMatrix4x4 m_matrix;
};

#endif // TILEDSTAGE_H
