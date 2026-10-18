#include "cannon.h"
#include "vid.h"
#include "map.h"
#include "constant.h"
#include "engine_globals.h"
#include "mylib.h"
#include <cmath>
#include <new>

namespace
{

bool CannonBetween(float value, float first, float second)
{
    if (first < second)
        return value >= first && value <= second;
    return value >= second && value <= first;
}
}

CANNON::CANNON(VID* vid, const VECTOR& position, ANGLE direction, SPRITE* parent)
    : SPRITE(vid, position, direction, parent), m_firstFlight(1u)
{
    if (m_vid->maxZSpeed != m_vid->randomZSpeed)
    {
        const float range = m_vid->randomZSpeed-m_vid->maxZSpeed;
        m_zSpeed = SPRITE::Random(range)+m_vid->maxZSpeed;
    }
    else if (parent && parent->Animation() >= 15 &&
        m_vid->topZ < 0.0f)
    {
        m_zSpeed = m_vid->maxZSpeed;
        m_speed = MaxSpeed()+parent->m_speed;
    }
    else
        m_zSpeed = m_vid->maxZSpeed;
    StartMove();
}

CANNON::~CANNON()
{
}

int CANNON::Action(int command, int argument1, int argument2, int argument3)
{
    if (command != ACT_NEXT_COMMAND)
        return SPRITE::Action(command, argument1, argument2, argument3);
    if (m_currentAnimation == 8)
        ChangeAnimation(15);
    else if (m_currentAnimation < 15)
    {
        if (m_xyz.z < -100.0f)
            ChangeAnimation(16);
        else if (m_speed != 0.0f)
            ChangeAnimation(2);
        else if (m_currentAnimation >= 7 && m_currentAnimation != 10)
            ChangeAnimation(0);
    }
    return 0;
}

void CANNON::DeletePointerToSprite(SPRITE* sprite)
{
    if (sprite && Goal() == sprite)
    {
        SPRITE* const replacement = new(std::nothrow) SPRITE(sprite->Coor(), ANGLE(0));
        SetGoal(replacement);
    }
    SPRITE::DeletePointerToSprite(sprite);
}

void CANNON::MoveTact()
{
    if (!m_vid->movementTactEnabled)
        return;

    VECTOR next;
    MoveTactCalcCoor(&next);
    if ((m_runtimeFlags & 0x1000u) != 0u && m_currentAnimation < 15)
        ChangeAnimation(15);

    const float groundHere = Map->GetGroundZ(VECTOR2(m_xyz.x, m_xyz.y));
    const float groundNext = Map->GetGroundZ(VECTOR2(next.x, next.y));
    const float topNext = groundNext+m_vid->topZ;
    const float oldZ = m_xyz.z;

    if (next.z <= groundNext &&
        m_xyz.z >= groundHere)
    {
        next.z = m_xyz.z;
        if (m_currentAnimation >= 15)
        {
            Stop();
            m_zSpeed = 0.0f;
            if (groundNext > groundHere)
                CreateChildFor(11, 0);
        }
        else if ((m_vid->property & 0x10000000u) == 0u)
        {
            Stop();
            m_zSpeed = 0.0f;
            CreateChildFor(groundNext > groundHere ? 11 : 12, 0);
            ChangeAnimation(15);
        }
        else if (groundNext > groundHere)
        {
            ChangeAnimation(11);
            ChangeDirection(m_direction.GetInversed());
        }
        else if (m_zSpeed < -0.022f)
        {
            ChangeAnimation(12);
            m_zSpeed = -m_zSpeed/2.0f;
        }
        else if (m_zSpeed > 0.005f)
        {
            m_zSpeed /= 2.0f;
            m_speed /= 2.0f;
            ChangeDirection(m_direction.GetInversed());
        }
        else
        {
            m_zSpeed = 0.0f;
            ChangeAnimation(15);
        }
        next.x = m_xyz.x;
        next.y = m_xyz.y;
    }
    else if (m_xyz.z != next.z &&
        (m_vid->property & 6u) == 0u &&
        m_vid->topZ != 0.0f)
    {
        if (m_xyz.z < topNext)
        {
            if (!(next.z < topNext))
            {
                next.z = topNext;
                m_zSpeed = 0.0f;
            }
        }
        else if (m_xyz.z > topNext)
        {
            if (!(next.z >= topNext))
            {
                next.z = topNext;
                m_zSpeed = 0.0f;
            }
        }
        else if ((m_vid->property & 0x08000000u) == 0u)
            m_zSpeed = 0.0f;
    }

    if (m_xyz.x != next.x || m_xyz.y != next.y)
    {
        if (m_currentAnimation < 15 && (next.XY()-m_xyz.XY()).NearLength() > 30.0f)
        {
            if ((next.XY()-m_xyz.XY()).NearLength() > 50.0f)
            {
                const VECTOR quarter = (next-m_xyz)*0.25f+m_xyz;
                if (CanPlaceWithCrush(quarter))
                {
                    ChangeAnimation(15);
                    goto cannon_move_height;
                }
                const VECTOR middle = (m_xyz+next)*0.5f;
                if (CanPlaceWithCrush(middle))
                {
                    ChangeCoor((next-m_xyz)*0.25f+m_xyz);
                    ChangeAnimation(15);
                    goto cannon_move_height;
                }
                const VECTOR threeQuarters = (next-m_xyz)*0.75f+m_xyz;
                if (CanPlaceWithCrush(threeQuarters))
                {
                    ChangeCoor((m_xyz+next)*0.5f);
                    ChangeAnimation(15);
                    goto cannon_move_height;
                }
                ChangeCoor((next-m_xyz)*0.75f+m_xyz);
            }
            else
            {
                const VECTOR middle = (m_xyz+next)*0.5f;
                if (CanPlaceWithCrush(middle))
                {
                    ChangeAnimation(15);
                    goto cannon_move_height;
                }
                ChangeCoor((m_xyz+next)*0.5f);
            }
        }
        if (CanPlaceWithCrush(next))
        {
            if (m_currentAnimation < 15)
                ChangeAnimation(15);
            next.z = m_xyz.z;
        }
        else if (!Map->InMap(VECTOR2(next.x, next.y)))
        {
            ChangeCoor(next);
            if (m_currentAnimation < 15 && (m_vid->property & 6u) == 0u)
            {
                const VECTOR2 extent = Map->SizeXY()+VECTOR2(200.0f, 200.0f);
                if (!(m_xyz.x > -200.0f && m_xyz.y > -200.0f &&
                    m_xyz.x <= extent.x &&
                    m_xyz.y <= extent.y))
                    ChangeAnimation(15);
            }
        }
        else
            ChangeCoor(next);
    }
    cannon_move_height:
    if (m_xyz.z != next.z)
        ChangeZCoor(next.z);

    if (!Goal())
    {
        if (m_vid->topZ < 0.0f &&
            m_zSpeed < 0.0f)
            RotateTact(m_direction+ANGLE(32),
                BitsToSigned32(::CurrentTime-::PrevCurrentTime));
        return;
    }

    const float distance = (VECTOR2(Goal()->X(), Goal()->Y())-
        VECTOR2(m_xyz.x, m_xyz.y)).NearLength();
    if ((m_vid->property & 0x08000000u) != 0u)
    {
        if (m_firstFlight)
        {
            const float ground = Map->GetGroundZ(VECTOR2(m_xyz.x, m_xyz.y));
            if ((ground+m_vid->topZ > m_xyz.z || Goal()->Z() > m_xyz.z) &&
                m_zSpeed > 0.0f)
                m_zSpeed -= (::CurrentTime-::PrevCurrentTime)*::Const->Gravitation;
            else
                m_firstFlight = 0u;
        }
        else
        {
            const ANGLE desired = DirectionTo(Goal())+
                ANGLE(m_speed < 0.0f ? 128 : 0);
            const ANGLE remaining = RotateTact(desired,
                BitsToSigned32(::CurrentTime-::PrevCurrentTime));
            if (distance > 10.0f && distance < 30.0f &&
                remaining.value > 70u)
            {
                m_firstFlight = 1u;
                StartMove();
            }
            else if (Goal()->Z() < m_xyz.z &&
                distance != 0.0f)
            {
                m_zSpeed = (Goal()->Z()-m_xyz.z)/distance/10.0f;
                if (-m_vid->maxZSpeed > m_zSpeed)
                    m_zSpeed = -m_vid->maxZSpeed;
            }
            else
                m_zSpeed = 0.0f;
        }
    }
    else if (distance > 100.0f)
    {
        const ANGLE oldDirection = m_direction;
        const ANGLE desired = DirectionTo(Goal())+
            ANGLE(m_speed < 0.0f ? 128 : 0);
        RotateTact(desired, BitsToSigned32(::CurrentTime-::PrevCurrentTime));
        if (AbsDiffU8(oldDirection.value, m_direction.value) > 100u)
        {
            Stop();
            m_zSpeed = 0.0f;
            ChangeAnimation(15);
        }
        return;
    }

    if (((m_runtimeFlags & 0x03000000u) == 0x03000000u ||
        IsXYCross(Goal()->Vid(), Goal()->Coor(), Goal()->Direction())) &&
        ((m_xyz.z+m_vid->sizeXYZ.z >= Goal()->Z() &&
          Goal()->Z()+Goal()->Vid()->sizeXYZ.z >= m_xyz.z) ||
         CannonBetween(Goal()->Z(), oldZ, m_xyz.z)))
    {
        Stop();
        m_zSpeed = 0.0f;
        ChangeAnimation(15);
    }
    else if ((m_vid->property & 0x08000000u) != 0u)
    {
        const VECTOR delta(Goal()->X()-m_xyz.x, Goal()->Y()-m_xyz.y,
            Goal()->Z()-m_xyz.z);
        if (std::fabs(delta.x) < 10.0f &&
            std::fabs(delta.y) < 10.0f &&
            std::fabs(delta.z) < 20.0f)
        {
            Stop();
            m_zSpeed = 0.0f;
            ChangeAnimation(15);
        }
    }
}
