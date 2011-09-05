#ifndef WORLD_H
#define WORLD_H

#include <btBulletDynamicsCommon.h>

class World {
public:
    World(int defaultFPS)
        : m_dispatcher(&m_config),
          m_broadphase(btVector3(-400.0f, -400.0f, -400.0f), btVector3(400.0f, 400.0, 400.0f), 1024),
          m_world(&m_dispatcher, &m_broadphase, &m_solver, &m_config)
    {
        m_world.setGravity(btVector3(0.0f, -9.8f * 2.0f, 0.0f));
        setCurrentFPS(defaultFPS);
    }
    ~World()
    {
    }

    btDiscreteDynamicsWorld *mutableWorld() {
        return &m_world;
    }
    void setCurrentFPS(int value) {
        m_world.getSolverInfo().m_numIterations = static_cast<int>(10.0f * 60.0f / value);
    }

private:
    btDefaultCollisionConfiguration m_config;
    btCollisionDispatcher m_dispatcher;
    btAxisSweep3 m_broadphase;
    btSequentialImpulseConstraintSolver m_solver;
    btDiscreteDynamicsWorld m_world;
};

#endif // WORLD_H
