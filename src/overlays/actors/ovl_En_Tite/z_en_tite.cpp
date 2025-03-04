#define INTERNAL_SRC_OVERLAYS_ACTORS_OVL_EN_TITE_Z_EN_TITE_C
#include "actor_common.h"
/*
 * File: z_en_tite.c
 * Overlay: ovl_En_Tite
 * Description: Tektite
 */

#include "z_en_tite.h"
#include "overlays/actors/ovl_En_Encount1/z_en_encount1.h"
#include "overlays/effects/ovl_Effect_Ss_Dead_Sound/z_eff_ss_dead_sound.h"
#include "vt.h"
#include "objects/object_tite/object_tite.h"
#include "def/random.h"
#include "def/sys_matrix.h"
#include "def/z_actor.h"
#include "def/z_bgcheck.h"
#include "def/z_collision_check.h"
#include "def/z_effect_soft_sprite_old_init.h"
#include "def/z_en_item00.h"
#include "def/z_lib.h"
#include "def/z_rcp.h"
#include "def/z_skelanime.h"

#define FLAGS (ACTOR_FLAG_0 | ACTOR_FLAG_2 | ACTOR_FLAG_4)

// EnTite_Idle
#define vIdleTimer actionVar1

// EnTite_Attack (vQueuedJumps also used by EnTite_MoveTowardPlayer)
#define vAttackState actionVar1
#define vQueuedJumps actionVar2

//  EnTite_FlipOnBack
#define vOnBackTimer actionVar1
#define vLegTwitchTimer actionVar2


void EnTite_Init(Actor* thisx, GlobalContext* globalCtx);
void EnTite_Reset(Actor* pthisx, GlobalContext* globalCtx);
void EnTite_Destroy(Actor* thisx, GlobalContext* globalCtx);
void EnTite_Update(Actor* thisx, GlobalContext* globalCtx);
void EnTite_Draw(Actor* thisx, GlobalContext* globalCtx);

void EnTite_SetupIdle(EnTite* pthis);
void EnTite_SetupTurnTowardPlayer(EnTite* pthis);
void EnTite_SetupMoveTowardPlayer(EnTite* pthis);
void EnTite_SetupDeathCry(EnTite* pthis);
void EnTite_SetupFlipUpright(EnTite* pthis);

void EnTite_Idle(EnTite* pthis, GlobalContext* globalCtx);
void EnTite_Attack(EnTite* pthis, GlobalContext* globalCtx);
void EnTite_TurnTowardPlayer(EnTite* pthis, GlobalContext* globalCtx);
void EnTite_MoveTowardPlayer(EnTite* pthis, GlobalContext* globalCtx);
void EnTite_Recoil(EnTite* pthis, GlobalContext* globalCtx);
void EnTite_Stunned(EnTite* pthis, GlobalContext* globalCtx);
void EnTite_DeathCry(EnTite* pthis, GlobalContext* globalCtx);
void EnTite_FallApart(EnTite* pthis, GlobalContext* globalCtx);
void EnTite_FlipOnBack(EnTite* pthis, GlobalContext* globalCtx);
void EnTite_FlipUpright(EnTite* pthis, GlobalContext* globalCtx);

ActorInit En_Tite_InitVars = {
    ACTOR_EN_TITE,
    ACTORCAT_ENEMY,
    FLAGS,
    OBJECT_TITE,
    sizeof(EnTite),
    (ActorFunc)EnTite_Init,
    (ActorFunc)EnTite_Destroy,
    (ActorFunc)EnTite_Update,
    (ActorFunc)EnTite_Draw,
    (ActorFunc)EnTite_Reset,
};

static ColliderJntSphElementInit sJntSphElementsInit[1] = {
    {
        {
            ELEMTYPE_UNK0,
            { 0xFFCFFFFF, 0x00, 0x08 },
            { 0xFFCFFFFF, 0x00, 0x00 },
            TOUCH_ON | TOUCH_SFX_NORMAL,
            BUMP_ON | BUMP_HOOKABLE,
            OCELEM_ON,
        },
        { 0, { { 0, 1500, 0 }, 20 }, 100 },
    },
};

static ColliderJntSphInit sJntSphInit = {
    {
        COLTYPE_HIT6,
        AT_ON | AT_TYPE_ENEMY,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_JNTSPH,
    },
    1,
    sJntSphElementsInit,
};

static DamageTable sDamageTable[] = {
    /* Deku nut      */ DMG_ENTRY(0, 0x1),
    /* Deku stick    */ DMG_ENTRY(2, 0x0),
    /* Slingshot     */ DMG_ENTRY(1, 0x0),
    /* Explosive     */ DMG_ENTRY(2, 0x0),
    /* Boomerang     */ DMG_ENTRY(0, 0x1),
    /* Normal arrow  */ DMG_ENTRY(2, 0x0),
    /* Hammer swing  */ DMG_ENTRY(2, 0x0),
    /* Hookshot      */ DMG_ENTRY(0, 0x1),
    /* Kokiri sword  */ DMG_ENTRY(1, 0x0),
    /* Master sword  */ DMG_ENTRY(2, 0x0),
    /* Giant's Knife */ DMG_ENTRY(4, 0x0),
    /* Fire arrow    */ DMG_ENTRY(2, 0x0),
    /* Ice arrow     */ DMG_ENTRY(4, 0xF),
    /* Light arrow   */ DMG_ENTRY(2, 0x0),
    /* Unk arrow 1   */ DMG_ENTRY(2, 0x0),
    /* Unk arrow 2   */ DMG_ENTRY(2, 0x0),
    /* Unk arrow 3   */ DMG_ENTRY(2, 0x0),
    /* Fire magic    */ DMG_ENTRY(0, 0xE),
    /* Ice magic     */ DMG_ENTRY(3, 0xF),
    /* Light magic   */ DMG_ENTRY(0, 0xE),
    /* Shield        */ DMG_ENTRY(0, 0x0),
    /* Mirror Ray    */ DMG_ENTRY(0, 0x0),
    /* Kokiri spin   */ DMG_ENTRY(1, 0x0),
    /* Giant spin    */ DMG_ENTRY(4, 0x0),
    /* Master spin   */ DMG_ENTRY(2, 0x0),
    /* Kokiri jump   */ DMG_ENTRY(2, 0x0),
    /* Giant jump    */ DMG_ENTRY(8, 0x0),
    /* Master jump   */ DMG_ENTRY(4, 0x0),
    /* Unknown 1     */ DMG_ENTRY(0, 0x0),
    /* Unblockable   */ DMG_ENTRY(0, 0x0),
    /* Hammer jump   */ DMG_ENTRY(4, 0x0),
    /* Unknown 2     */ DMG_ENTRY(0, 0x0),
};

static InitChainEntry sInitChain[] = {
    ICHAIN_S8(naviEnemyId, 0x45, ICHAIN_CONTINUE),
    ICHAIN_F32(targetArrowOffset, 2000, ICHAIN_CONTINUE),
    ICHAIN_F32(minVelocityY, -40, ICHAIN_CONTINUE),
    ICHAIN_F32_DIV1000(gravity, -1000, ICHAIN_STOP),
};

static AnimationHeader* D_80B1B634[] = {
    &object_tite_Anim_00083C, &object_tite_Anim_0004F8, &object_tite_Anim_00069C, NULL, NULL, NULL,
};

// Some kind of offset for the position of each tektite foot
static Vec3f sFootOffset = { 2800.0f, -200.0f, 0.0f };

// Relative positions to spawn ice chunks when tektite is frozen
static Vec3f sIceChunks[12] = {
    { 20.0f, 20.0f, 0.0f },   { 10.0f, 40.0f, 10.0f },   { -10.0f, 40.0f, 10.0f }, { -20.0f, 20.0f, 0.0f },
    { 10.0f, 40.0f, -10.0f }, { -10.0f, 40.0f, -10.0f }, { 0.0f, 20.0f, -20.0f },  { 10.0f, 0.0f, 10.0f },
    { 10.0f, 0.0f, -10.0f },  { 0.0f, 20.0f, 20.0f },    { -10.0f, 0.0f, 10.0f },  { -10.0f, 0.0f, -10.0f },
};

void EnTite_SetupAction(EnTite* pthis, EnTiteActionFunc actionFunc) {
    pthis->actionFunc = actionFunc;
}

void EnTite_Init(Actor* thisx, GlobalContext* globalCtx) {
    EnTite* pthis = (EnTite*)thisx;

    Actor_ProcessInitChain(thisx, sInitChain);
    thisx->targetMode = 3;
    Actor_SetScale(thisx, 0.01f);
    SkelAnime_Init(globalCtx, &pthis->skelAnime, &object_tite_Skel_003A20, &object_tite_Anim_0012E4, pthis->jointTable,
                   pthis->morphTable, 25);
    ActorShape_Init(&thisx->shape, -200.0f, ActorShadow_DrawCircle, 70.0f);
    pthis->flipState = TEKTITE_INITIAL;
    thisx->colChkInfo.damageTable = sDamageTable;
    pthis->actionVar1 = 0;
    pthis->bodyBreak.val = BODYBREAK_STATUS_FINISHED;
    thisx->focus.pos = thisx->world.pos;
    thisx->focus.pos.y += 20.0f;
    thisx->colChkInfo.health = 2;
    thisx->colChkInfo.mass = MASS_HEAVY;
    Collider_InitJntSph(globalCtx, &pthis->collider);
    Collider_SetJntSph(globalCtx, &pthis->collider, thisx, &sJntSphInit, &pthis->colliderItem);
    pthis->unk_2DC = 0x1D;
    if (pthis->actor.params == TEKTITE_BLUE) {
        pthis->unk_2DC |= 0x40; // Don't use the actor engine's ripple spawning code
        thisx->colChkInfo.health = 4;
        thisx->naviEnemyId += 1;
    }
    EnTite_SetupIdle(pthis);
}

void EnTite_Destroy(Actor* thisx, GlobalContext* globalCtx) {
    EnTite* pthis = (EnTite*)thisx;
    EnEncount1* spawner;

    if (thisx->parent != NULL) {
        spawner = (EnEncount1*)thisx->parent;
        if (spawner->curNumSpawn > 0) {
            spawner->curNumSpawn--;
        }
        osSyncPrintf("\n\n");
        // "Number of simultaneous occurrences"
        osSyncPrintf(VT_FGCOL(GREEN) "☆☆☆☆☆ 同時発生数 ☆☆☆☆☆%d\n" VT_RST, spawner->curNumSpawn);
        osSyncPrintf("\n\n");
    }
    Collider_DestroyJntSph(globalCtx, &pthis->collider);
}

void EnTite_SetupIdle(EnTite* pthis) {
    Animation_MorphToLoop(&pthis->skelAnime, &object_tite_Anim_0012E4, 4.0f);
    pthis->action = TEKTITE_IDLE;
    pthis->vIdleTimer = Rand_S16Offset(15, 30);
    pthis->actor.speedXZ = 0.0f;
    EnTite_SetupAction(pthis, EnTite_Idle);
}

void EnTite_Idle(EnTite* pthis, GlobalContext* globalCtx) {
    SkelAnime_Update(&pthis->skelAnime);
    Math_SmoothStepToF(&pthis->actor.speedXZ, 0.0f, 1.0f, 0.5f, 0.0f);
    if (pthis->actor.params == TEKTITE_BLUE) {
        if (pthis->actor.bgCheckFlags & 0x20) {
            // Float on water surface
            pthis->actor.gravity = 0.0f;
            Math_SmoothStepToF(&pthis->actor.velocity.y, 0.0f, 1.0f, 2.0f, 0.0f);
            Math_SmoothStepToF(&pthis->actor.world.pos.y, pthis->actor.world.pos.y + pthis->actor.yDistToWater, 1.0f, 2.0f,
                               0.0f);
        } else {
            pthis->actor.gravity = -1.0f;
        }
    }
    if ((pthis->actor.bgCheckFlags & 3) && (pthis->actor.velocity.y <= 0.0f)) {
        pthis->actor.velocity.y = 0.0f;
    }
    if (pthis->vIdleTimer > 0) {
        pthis->vIdleTimer--;
    } else if ((pthis->actor.xzDistToPlayer < 300.0f) && (pthis->actor.yDistToPlayer <= 80.0f)) {
        EnTite_SetupTurnTowardPlayer(pthis);
    }
}

void EnTite_SetupAttack(EnTite* pthis) {
    Animation_PlayOnce(&pthis->skelAnime, &object_tite_Anim_00083C);
    pthis->action = TEKTITE_ATTACK;
    pthis->vAttackState = TEKTITE_BEGIN_LUNGE;
    pthis->vQueuedJumps = Rand_S16Offset(1, 3);
    pthis->actor.speedXZ = 0.0f;
    pthis->actor.velocity.y = 0.0f;
    pthis->actor.world.rot.y = pthis->actor.shape.rot.y;
    EnTite_SetupAction(pthis, EnTite_Attack);
}

void EnTite_Attack(EnTite* pthis, GlobalContext* globalCtx) {
    s16 angleToPlayer;
    s32 attackState;
    Vec3f ripplePos;

    if (SkelAnime_Update(&pthis->skelAnime) != 0) {
        attackState = pthis->vAttackState; // for deciding whether to change animation
        switch (pthis->vAttackState) {
            case TEKTITE_BEGIN_LUNGE:
                // Snap to ground or water, then lunge into the air with some initial speed
                pthis->vAttackState = TEKTITE_MID_LUNGE;
                if ((pthis->actor.params != TEKTITE_BLUE) || !(pthis->actor.bgCheckFlags & 0x20)) {
                    if (pthis->actor.floorHeight > BGCHECK_Y_MIN) {
                        pthis->actor.world.pos.y = pthis->actor.floorHeight;
                    }
                    Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_STAL_JUMP);
                } else {
                    pthis->actor.world.pos.y += pthis->actor.yDistToWater;
                    Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_TEKU_JUMP_WATER);
                }
                pthis->actor.velocity.y = 8.0f;
                pthis->actor.gravity = -1.0f;
                pthis->actor.speedXZ = 4.0f;
                break;
            case TEKTITE_MID_LUNGE:
                // Continue trajectory until tektite has negative velocity and has landed on ground/water surface
                // Snap to ground/water surface, or if falling fast dip into the water and slow fall speed
                pthis->actor.flags |= ACTOR_FLAG_24;
                if ((pthis->actor.bgCheckFlags & 3) ||
                    ((pthis->actor.params == TEKTITE_BLUE) && (pthis->actor.bgCheckFlags & 0x20))) {
                    if (pthis->actor.velocity.y <= 0.0f) {
                        pthis->vAttackState = TEKTITE_LANDED;
                        if ((pthis->actor.params != TEKTITE_BLUE) || !(pthis->actor.bgCheckFlags & 0x20)) {
                            if (BGCHECK_Y_MIN < pthis->actor.floorHeight) {
                                pthis->actor.world.pos.y = pthis->actor.floorHeight;
                            }
                            pthis->actor.velocity.y = 0.0f;
                            pthis->actor.speedXZ = 0.0f;
                        } else {
                            pthis->actor.gravity = 0.0f;
                            if (pthis->actor.velocity.y < -8.0f) {
                                ripplePos = pthis->actor.world.pos;
                                ripplePos.y += pthis->actor.yDistToWater;
                                pthis->vAttackState++; // TEKTITE_SUBMERGED
                                pthis->actor.velocity.y *= 0.75f;
                                attackState = pthis->vAttackState;
                                EffectSsGRipple_Spawn(globalCtx, &ripplePos, 0, 500, 0);
                            } else {
                                pthis->actor.velocity.y = 0.0f;
                                pthis->actor.speedXZ = 0.0f;
                            }
                        }
                        pthis->actor.world.rot.y = pthis->actor.shape.rot.y;
                    }
                }
                break;
            case TEKTITE_LANDED:
                // Get ready to begin another lunge if more lunges are queued, otherwise start turning
                if (pthis->vQueuedJumps != 0) {
                    pthis->vQueuedJumps--;
                    pthis->vAttackState = TEKTITE_BEGIN_LUNGE;
                    pthis->collider.base.atFlags &= ~AT_HIT;
                } else {
                    EnTite_SetupTurnTowardPlayer(pthis);
                }
                break;
            case TEKTITE_SUBMERGED:
                // Check if floated to surface
                if (pthis->actor.yDistToWater == 0.0f) {
                    pthis->vAttackState = TEKTITE_LANDED;
                    attackState = pthis->vAttackState;
                }
                break;
        }
        // If switching attack state, change animation (unless tektite is switching between submerged and landed)
        if (attackState != pthis->vAttackState) {
            Animation_PlayOnce(&pthis->skelAnime, D_80B1B634[pthis->vAttackState]);
        }
    }

    switch (pthis->vAttackState) {
        case TEKTITE_BEGIN_LUNGE:
            // Slightly turn to player and switch to turning/idling action if the player is too far
            Math_SmoothStepToS(&pthis->actor.world.rot.y, pthis->actor.yawTowardsPlayer, 1, 1000, 0);
            pthis->actor.shape.rot.y = pthis->actor.world.rot.y;
            angleToPlayer = pthis->actor.yawTowardsPlayer - pthis->actor.shape.rot.y;
            if ((pthis->actor.xzDistToPlayer > 300.0f) && (pthis->actor.yDistToPlayer > 80.0f)) {
                EnTite_SetupIdle(pthis);
            } else if (ABS(angleToPlayer) >= 9000) {
                EnTite_SetupTurnTowardPlayer(pthis);
            }
            break;
        case TEKTITE_MID_LUNGE:
            // Generate sparkles at feet upon landing, set jumping animation and hurtbox and check if hit player
            if (pthis->actor.velocity.y >= 5.0f) {
                if (pthis->actor.bgCheckFlags & 1) {
                    func_800355B8(globalCtx, &pthis->frontLeftFootPos);
                    func_800355B8(globalCtx, &pthis->frontRightFootPos);
                    func_800355B8(globalCtx, &pthis->backRightFootPos);
                    func_800355B8(globalCtx, &pthis->backLeftFootPos);
                }
            }
            if (!(pthis->collider.base.atFlags & AT_HIT) && (pthis->actor.flags & ACTOR_FLAG_UNCULLED)) {
                CollisionCheck_SetAT(globalCtx, &globalCtx->colChkCtx, &pthis->collider.base);
            } else {
                Player* player = GET_PLAYER(globalCtx);
                pthis->collider.base.atFlags &= ~AT_HIT;
                Animation_MorphToLoop(&pthis->skelAnime, &object_tite_Anim_0012E4, 4.0f);
                pthis->actor.speedXZ = -6.0f;
                pthis->actor.world.rot.y = pthis->actor.yawTowardsPlayer;
                if (&player->actor == pthis->collider.base.at) {
                    if (!(pthis->collider.base.atFlags & AT_BOUNCED)) {
                        Audio_PlayActorSound2(&player->actor, NA_SE_PL_BODY_HIT);
                    }
                }
                EnTite_SetupAction(pthis, EnTite_Recoil);
            }
            break;
        case TEKTITE_LANDED:
            // Slightly turn to player
            Math_SmoothStepToS(&pthis->actor.world.rot.y, pthis->actor.yawTowardsPlayer, 1, 1500, 0);
            break;
        case TEKTITE_SUBMERGED:
            // Float up to water surface
            Math_SmoothStepToF(&pthis->actor.velocity.y, 0.0f, 1.0f, 2.0f, 0.0f);
            Math_SmoothStepToF(&pthis->actor.speedXZ, 0.0f, 1.0f, 0.5f, 0.0f);
            Math_SmoothStepToF(&pthis->actor.world.pos.y, pthis->actor.world.pos.y + pthis->actor.yDistToWater, 1.0f, 2.0f,
                               0.0f);
            break;
    }
    // Create ripples on water surface where tektite feet landed
    if (pthis->actor.bgCheckFlags & 2) {
        if (!(pthis->actor.bgCheckFlags & 0x20)) {
            func_80033480(globalCtx, &pthis->frontLeftFootPos, 1.0f, 2, 80, 15, 1);
            func_80033480(globalCtx, &pthis->frontRightFootPos, 1.0f, 2, 80, 15, 1);
            func_80033480(globalCtx, &pthis->backRightFootPos, 1.0f, 2, 80, 15, 1);
            func_80033480(globalCtx, &pthis->backLeftFootPos, 1.0f, 2, 80, 15, 1);
        }
    }
    // if landed, kill XZ speed and play appropriate sounds
    if (pthis->actor.params == TEKTITE_BLUE) {
        if (pthis->actor.bgCheckFlags & 0x40) {
            pthis->actor.speedXZ = 0.0f;
            if (pthis->vAttackState == TEKTITE_SUBMERGED) {
                Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_TEKU_LAND_WATER);
            } else {
                Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_TEKU_LAND_WATER2);
            }
            pthis->actor.bgCheckFlags &= ~0x40;
        } else if (pthis->actor.bgCheckFlags & 2) {
            Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_DODO_M_GND);
        }
    } else if (pthis->actor.bgCheckFlags & 2) {
        pthis->actor.speedXZ = 0.0f;
        Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_DODO_M_GND);
    }
}

void EnTite_SetupTurnTowardPlayer(EnTite* pthis) {
    Animation_PlayLoop(&pthis->skelAnime, &object_tite_Anim_000A14);
    pthis->action = TEKTITE_TURN_TOWARD_PLAYER;
    if ((pthis->actor.bgCheckFlags & 3) || ((pthis->actor.params == TEKTITE_BLUE) && (pthis->actor.bgCheckFlags & 0x20))) {
        if (pthis->actor.velocity.y <= 0.0f) {
            pthis->actor.gravity = 0.0f;
            pthis->actor.velocity.y = 0.0f;
            pthis->actor.speedXZ = 0.0f;
        }
    }
    EnTite_SetupAction(pthis, EnTite_TurnTowardPlayer);
}

void EnTite_TurnTowardPlayer(EnTite* pthis, GlobalContext* globalCtx) {
    s16 angleToPlayer;
    s16 turnVelocity;

    if (((pthis->actor.bgCheckFlags & 3) ||
         ((pthis->actor.params == TEKTITE_BLUE) && (pthis->actor.bgCheckFlags & 0x20))) &&
        (pthis->actor.velocity.y <= 0.0f)) {
        pthis->actor.gravity = 0.0f;
        pthis->actor.velocity.y = 0.0f;
        pthis->actor.speedXZ = 0.0f;
    }
    // Calculate turn velocity and animation speed based on angle to player
    if ((pthis->actor.params == TEKTITE_BLUE) && (pthis->actor.bgCheckFlags & 0x20)) {
        pthis->actor.world.pos.y += pthis->actor.yDistToWater;
    }
    angleToPlayer = Actor_WorldYawTowardActor(&pthis->actor, &GET_PLAYER(globalCtx)->actor) - pthis->actor.world.rot.y;
    if (angleToPlayer > 0) {
        turnVelocity = (angleToPlayer / 42.0f) + 10.0f;
        pthis->actor.world.rot.y += (turnVelocity * 2);
    } else {
        turnVelocity = (angleToPlayer / 42.0f) - 10.0f;
        pthis->actor.world.rot.y += (turnVelocity * 2);
    }
    if (angleToPlayer > 0) {
        pthis->skelAnime.playSpeed = turnVelocity * 0.01f;
    } else {
        pthis->skelAnime.playSpeed = turnVelocity * 0.01f;
    }

    /**
     * Play sounds once every animation cycle
     */
    SkelAnime_Update(&pthis->skelAnime);
    if (((s16)pthis->skelAnime.curFrame & 7) == 0) {
        if ((pthis->actor.params == TEKTITE_BLUE) && (pthis->actor.bgCheckFlags & 0x20)) {
            Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_TEKU_WALK_WATER);
        } else {
            Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_TEKU_WALK);
        }
    }

    // Idle if player is far enough away from the tektite, move or attack if almost facing player
    pthis->actor.shape.rot.y = pthis->actor.world.rot.y;
    if ((pthis->actor.xzDistToPlayer > 300.0f) && (pthis->actor.yDistToPlayer > 80.0f)) {
        EnTite_SetupIdle(pthis);
    } else if (Actor_IsFacingPlayer(&pthis->actor, 3640)) {
        if ((pthis->actor.xzDistToPlayer <= 180.0f) && (pthis->actor.yDistToPlayer <= 80.0f)) {
            EnTite_SetupAttack(pthis);
        } else {
            EnTite_SetupMoveTowardPlayer(pthis);
        }
    }
}

void EnTite_SetupMoveTowardPlayer(EnTite* pthis) {
    Animation_PlayLoop(&pthis->skelAnime, &object_tite_Anim_000C70);
    pthis->action = TEKTITE_MOVE_TOWARD_PLAYER;
    pthis->actor.velocity.y = 10.0f;
    pthis->actor.gravity = -1.0f;
    pthis->actor.speedXZ = 4.0f;
    pthis->vQueuedJumps = Rand_S16Offset(1, 3);
    if ((pthis->actor.params == TEKTITE_BLUE) && (pthis->actor.bgCheckFlags & 0x20)) {
        Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_TEKU_JUMP_WATER);
    } else {
        Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_STAL_JUMP);
    }
    EnTite_SetupAction(pthis, EnTite_MoveTowardPlayer);
}

/**
 *  Jumping toward player as a method of travel (different from attacking, has no hitbox)
 */
void EnTite_MoveTowardPlayer(EnTite* pthis, GlobalContext* globalCtx) {
    Math_SmoothStepToF(&pthis->actor.speedXZ, 0.0f, 0.1f, 1.0f, 0.0f);
    SkelAnime_Update(&pthis->skelAnime);

    if (pthis->actor.bgCheckFlags & 0x42) {
        if (!(pthis->actor.bgCheckFlags & 0x40)) {
            func_80033480(globalCtx, &pthis->frontLeftFootPos, 1.0f, 2, 80, 15, 1);
            func_80033480(globalCtx, &pthis->frontRightFootPos, 1.0f, 2, 80, 15, 1);
            func_80033480(globalCtx, &pthis->backRightFootPos, 1.0f, 2, 80, 15, 1);
            func_80033480(globalCtx, &pthis->backLeftFootPos, 1.0f, 2, 80, 15, 1);
            Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_DODO_M_GND);
        } else {
            Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_TEKU_LAND_WATER);
        }
    }

    if ((pthis->actor.bgCheckFlags & 2) || ((pthis->actor.params == TEKTITE_BLUE) && (pthis->actor.bgCheckFlags & 0x40))) {
        if (pthis->vQueuedJumps != 0) {
            pthis->vQueuedJumps--;
        } else {
            EnTite_SetupIdle(pthis);
        }
    }

    if (((pthis->actor.bgCheckFlags & 3) || (pthis->actor.params == TEKTITE_BLUE && (pthis->actor.bgCheckFlags & 0x60))) &&
        (pthis->actor.velocity.y <= 0.0f)) {
        // slightly turn toward player upon landing and snap to ground or water.
        pthis->actor.speedXZ = 0.0f;
        Math_SmoothStepToS(&pthis->actor.shape.rot.y, pthis->actor.yawTowardsPlayer, 1, 4000, 0);
        pthis->actor.world.rot.y = pthis->actor.shape.rot.y;
        if ((pthis->actor.params != TEKTITE_BLUE) || !(pthis->actor.bgCheckFlags & 0x20)) {
            if (pthis->actor.floorHeight > BGCHECK_Y_MIN) {
                pthis->actor.world.pos.y = pthis->actor.floorHeight;
            }
        } else if (pthis->actor.bgCheckFlags & 0x40) {
            Vec3f ripplePos = pthis->actor.world.pos;
            pthis->actor.bgCheckFlags &= ~0x40;
            ripplePos.y += pthis->actor.yDistToWater;
            pthis->actor.gravity = 0.0f;
            pthis->actor.velocity.y *= 0.75f;
            EffectSsGRipple_Spawn(globalCtx, &ripplePos, 0, 500, 0);
            return;
        } else {
            // If submerged, float to surface
            Math_SmoothStepToF(&pthis->actor.velocity.y, 0.0f, 1.0f, 2.0f, 0.0f);
            Math_SmoothStepToF(&pthis->actor.world.pos.y, pthis->actor.world.pos.y + pthis->actor.yDistToWater, 1.0f, 2.0f,
                               0.0f);
            if (pthis->actor.yDistToWater != 0.0f) {
                // Do not change state until tekite has floated to surface
                return;
            }
        }

        // Idle or turn if player is too far away, otherwise keep jumping
        if (((pthis->actor.xzDistToPlayer > 300.0f) && (pthis->actor.yDistToPlayer > 80.0f))) {
            EnTite_SetupIdle(pthis);
        } else if (((pthis->actor.xzDistToPlayer <= 180.0f)) && ((pthis->actor.yDistToPlayer <= 80.0f))) {
            if (pthis->vQueuedJumps <= 0) {
                EnTite_SetupTurnTowardPlayer(pthis);
            } else {
                pthis->actor.velocity.y = 10.0f;
                pthis->actor.speedXZ = 4.0f;
                pthis->actor.flags |= ACTOR_FLAG_24;
                pthis->actor.gravity = -1.0f;
                if ((pthis->actor.params == TEKTITE_BLUE) && (pthis->actor.bgCheckFlags & 0x20)) {
                    Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_TEKU_JUMP_WATER);
                } else {
                    Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_STAL_JUMP);
                }
            }
        } else {
            pthis->actor.velocity.y = 10.0f;
            pthis->actor.speedXZ = 4.0f;
            pthis->actor.flags |= ACTOR_FLAG_24;
            pthis->actor.gravity = -1.0f;
            if ((pthis->actor.params == TEKTITE_BLUE) && (pthis->actor.bgCheckFlags & 0x20)) {
                Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_TEKU_JUMP_WATER);
            } else {
                Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_STAL_JUMP);
            }
        }
        // If in midair:
    } else {
        // Turn slowly toward player
        pthis->actor.flags |= ACTOR_FLAG_24;
        Math_SmoothStepToS(&pthis->actor.shape.rot.y, pthis->actor.yawTowardsPlayer, 1, 1000, 0);
        if (pthis->actor.velocity.y >= 6.0f) {
            if (pthis->actor.bgCheckFlags & 1) {
                func_800355B8(globalCtx, &pthis->frontLeftFootPos);
                func_800355B8(globalCtx, &pthis->frontRightFootPos);
                func_800355B8(globalCtx, &pthis->backRightFootPos);
                func_800355B8(globalCtx, &pthis->backLeftFootPos);
            }
        }
    }
}

void EnTite_SetupRecoil(EnTite* pthis) {
    pthis->action = TEKTITE_RECOIL;
    Animation_MorphToLoop(&pthis->skelAnime, &object_tite_Anim_0012E4, 4.0f);
    pthis->actor.speedXZ = -6.0f;
    pthis->actor.world.rot.y = pthis->actor.yawTowardsPlayer;
    pthis->actor.gravity = -1.0f;
    EnTite_SetupAction(pthis, EnTite_Recoil);
}

/**
 * After tektite hits or gets hit, recoils backwards and slides a bit upon landing
 */
void EnTite_Recoil(EnTite* pthis, GlobalContext* globalCtx) {
    s16 angleToPlayer;

    // Snap to ground or water surface upon landing
    Math_SmoothStepToF(&pthis->actor.speedXZ, 0.0f, 1.0f, 0.5f, 0.0f);
    if (((pthis->actor.bgCheckFlags & 3) || (pthis->actor.params == TEKTITE_BLUE && (pthis->actor.bgCheckFlags & 0x20))) &&
        (pthis->actor.velocity.y <= 0.0f)) {
        if ((pthis->actor.params != TEKTITE_BLUE) || !(pthis->actor.bgCheckFlags & 0x20)) {
            if (pthis->actor.floorHeight > BGCHECK_Y_MIN) {
                pthis->actor.world.pos.y = pthis->actor.floorHeight;
            }
        } else {
            pthis->actor.velocity.y = 0.0f;
            pthis->actor.gravity = 0.0f;
            pthis->actor.world.pos.y += pthis->actor.yDistToWater;
        }
    }

    // play sound and generate ripples
    if (pthis->actor.bgCheckFlags & 0x42) {
        if (!(pthis->actor.bgCheckFlags & 0x40)) {
            func_80033480(globalCtx, &pthis->frontLeftFootPos, 1.0f, 2, 80, 15, 1);
            func_80033480(globalCtx, &pthis->frontRightFootPos, 1.0f, 2, 80, 15, 1);
            func_80033480(globalCtx, &pthis->backRightFootPos, 1.0f, 2, 80, 15, 1);
            func_80033480(globalCtx, &pthis->backLeftFootPos, 1.0f, 2, 80, 15, 1);
            Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_DODO_M_GND);
        } else {
            pthis->actor.bgCheckFlags &= ~0x40;
            Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_TEKU_LAND_WATER2);
        }
    }

    // If player is far away, idle. Otherwise attack or move
    angleToPlayer = (pthis->actor.yawTowardsPlayer - pthis->actor.shape.rot.y);
    if ((pthis->actor.speedXZ == 0.0f) && ((pthis->actor.bgCheckFlags & 1) || ((pthis->actor.params == TEKTITE_BLUE) &&
                                                                             (pthis->actor.bgCheckFlags & 0x20)))) {
        pthis->actor.world.rot.y = pthis->actor.shape.rot.y;
        pthis->collider.base.atFlags &= ~AT_HIT;
        if ((pthis->actor.xzDistToPlayer > 300.0f) && (pthis->actor.yDistToPlayer > 80.0f) &&
            (ABS(pthis->actor.shape.rot.x) < 4000) && (ABS(pthis->actor.shape.rot.z) < 4000) &&
            ((pthis->actor.bgCheckFlags & 1) ||
             ((pthis->actor.params == TEKTITE_BLUE) && (pthis->actor.bgCheckFlags & 0x20)))) {
            EnTite_SetupIdle(pthis);
        } else if ((pthis->actor.xzDistToPlayer < 180.0f) && (pthis->actor.yDistToPlayer <= 80.0f) &&
                   (ABS(angleToPlayer) <= 6000)) {
            EnTite_SetupAttack(pthis);
        } else {
            EnTite_SetupMoveTowardPlayer(pthis);
        }
    }
    SkelAnime_Update(&pthis->skelAnime);
}

void EnTite_SetupStunned(EnTite* pthis) {
    Animation_Change(&pthis->skelAnime, &object_tite_Anim_0012E4, 0.0f, 0.0f,
                     (f32)Animation_GetLastFrame(&object_tite_Anim_0012E4), ANIMMODE_LOOP, 4.0f);
    pthis->action = TEKTITE_STUNNED;
    pthis->actor.speedXZ = -6.0f;
    pthis->actor.world.rot.y = pthis->actor.yawTowardsPlayer;
    if (pthis->damageEffect == 0xF) {
        pthis->spawnIceTimer = 48;
    }
    Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_GOMA_JR_FREEZE);
    EnTite_SetupAction(pthis, EnTite_Stunned);
}

/**
 * stunned or frozen
 */
void EnTite_Stunned(EnTite* pthis, GlobalContext* globalCtx) {
    s16 angleToPlayer;

    Math_SmoothStepToF(&pthis->actor.speedXZ, 0.0f, 1.0f, 0.5f, 0.0f);
    // Snap to ground or water
    if (((pthis->actor.bgCheckFlags & 3) ||
         ((pthis->actor.params == TEKTITE_BLUE) && (pthis->actor.bgCheckFlags & 0x20))) &&
        (pthis->actor.velocity.y <= 0.0f)) {
        if (((pthis->actor.params != TEKTITE_BLUE) || !(pthis->actor.bgCheckFlags & 0x20))) {
            if (pthis->actor.floorHeight > BGCHECK_Y_MIN) {
                pthis->actor.world.pos.y = pthis->actor.floorHeight;
            }
        } else {
            pthis->actor.velocity.y = 0.0f;
            pthis->actor.gravity = 0.0f;
            pthis->actor.world.pos.y += pthis->actor.yDistToWater;
        }
    }
    // Play sounds and spawn dirt effects upon landing
    if (pthis->actor.bgCheckFlags & 0x42) {
        if (!(pthis->actor.bgCheckFlags & 0x40)) {
            func_80033480(globalCtx, &pthis->frontLeftFootPos, 1.0f, 2, 80, 15, 1);
            func_80033480(globalCtx, &pthis->frontRightFootPos, 1.0f, 2, 80, 15, 1);
            func_80033480(globalCtx, &pthis->backRightFootPos, 1.0f, 2, 80, 15, 1);
            func_80033480(globalCtx, &pthis->backLeftFootPos, 1.0f, 2, 80, 15, 1);
            Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_DODO_M_GND);
        } else {
            pthis->actor.bgCheckFlags &= ~0x40;
            Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_TEKU_LAND_WATER2);
        }
    }
    // Decide on next action based on health, flip state and player distance
    angleToPlayer = pthis->actor.yawTowardsPlayer - pthis->actor.shape.rot.y;
    if (((pthis->actor.colorFilterTimer == 0) && (pthis->actor.speedXZ == 0.0f)) &&
        ((pthis->actor.bgCheckFlags & 1) ||
         ((pthis->actor.params == TEKTITE_BLUE) && (pthis->actor.bgCheckFlags & 0x20)))) {
        pthis->actor.world.rot.y = pthis->actor.shape.rot.y;
        if (pthis->actor.colChkInfo.health == 0) {
            EnTite_SetupDeathCry(pthis);
        } else if (pthis->flipState == TEKTITE_FLIPPED) {
            EnTite_SetupFlipUpright(pthis);
        } else if (((pthis->actor.xzDistToPlayer > 300.0f) && (pthis->actor.yDistToPlayer > 80.0f) &&
                    (ABS(pthis->actor.shape.rot.x) < 4000) && (ABS(pthis->actor.shape.rot.z) < 4000)) &&
                   ((pthis->actor.bgCheckFlags & 1) ||
                    ((pthis->actor.params == TEKTITE_BLUE) && (pthis->actor.bgCheckFlags & 0x20)))) {
            EnTite_SetupIdle(pthis);
        } else if ((pthis->actor.xzDistToPlayer < 180.0f) && (pthis->actor.yDistToPlayer <= 80.0f) &&
                   (ABS(angleToPlayer) <= 6000)) {
            EnTite_SetupAttack(pthis);
        } else {
            EnTite_SetupMoveTowardPlayer(pthis);
        }
    }
    SkelAnime_Update(&pthis->skelAnime);
}

void EnTite_SetupDeathCry(EnTite* pthis) {
    pthis->action = TEKTITE_DEATH_CRY;
    pthis->actor.colorFilterTimer = 0;
    pthis->actor.speedXZ = 0.0f;
    EnTite_SetupAction(pthis, EnTite_DeathCry);
}

/**
 * First frame of death. Scream in pain and allocate memory for EnPart data
 */
void EnTite_DeathCry(EnTite* pthis, GlobalContext* globalCtx) {
    EffectSsDeadSound_SpawnStationary(globalCtx, &pthis->actor.projectedPos, NA_SE_EN_TEKU_DEAD, true,
                                      DEADSOUND_REPEAT_MODE_OFF, 40);
    pthis->action = TEKTITE_FALL_APART;
    EnTite_SetupAction(pthis, EnTite_FallApart);
    BodyBreak_Alloc(&pthis->bodyBreak, 24, globalCtx);
}

/**
 * Spawn EnPart and drop items
 */
void EnTite_FallApart(EnTite* pthis, GlobalContext* globalCtx) {
    if (BodyBreak_SpawnParts(&pthis->actor, &pthis->bodyBreak, globalCtx, pthis->actor.params + 0xB)) {
        if (pthis->actor.params == TEKTITE_BLUE) {
            Item_DropCollectibleRandom(globalCtx, &pthis->actor, &pthis->actor.world.pos, 0xE0);
        } else {
            Item_DropCollectibleRandom(globalCtx, &pthis->actor, &pthis->actor.world.pos, 0x40);
        }
        Actor_Kill(&pthis->actor);
    }
}

void EnTite_SetupFlipOnBack(EnTite* pthis) {

    Animation_PlayLoopSetSpeed(&pthis->skelAnime, &object_tite_Anim_000A14, 1.5f);
    Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_TEKU_REVERSE);
    pthis->flipState = TEKTITE_FLIPPED;
    pthis->vOnBackTimer = 500;
    pthis->actor.speedXZ = 0.0f;
    pthis->actor.gravity = -1.0f;
    pthis->vLegTwitchTimer = (Rand_ZeroOne() * 50.0f);
    pthis->actor.velocity.y = 11.0f;
    EnTite_SetupAction(pthis, EnTite_FlipOnBack);
}

/**
 * During the flip animation and also while idling on back
 */
void EnTite_FlipOnBack(EnTite* pthis, GlobalContext* globalCtx) {
    Math_SmoothStepToS(&pthis->actor.shape.rot.z, 0x7FFF, 1, 4000, 0);
    // randomly reset the leg wiggling animation whenever timer reaches 0 to give illusion of twitching legs
    pthis->vLegTwitchTimer--;
    if (pthis->vLegTwitchTimer == 0) {
        pthis->vLegTwitchTimer = Rand_ZeroOne() * 30.0f;
        pthis->skelAnime.curFrame = Rand_ZeroOne() * 5.0f;
    }
    SkelAnime_Update(&pthis->skelAnime);
    if (pthis->actor.bgCheckFlags & 3) {
        // Upon landing, spawn dust and make noise
        if (pthis->actor.bgCheckFlags & 2) {
            Actor_SpawnFloorDustRing(globalCtx, &pthis->actor, &pthis->actor.world.pos, 20.0f, 0xB, 4.0f, 0, 0, 0);
            Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_DODO_M_GND);
        }
        pthis->vOnBackTimer--;
        if (pthis->vOnBackTimer == 0) {
            EnTite_SetupFlipUpright(pthis);
        }
    } else {
        // Gradually increase y offset during flip so that the actor position is at tektite's back instead of feet
        if (pthis->actor.shape.yOffset < 2800.0f) {
            pthis->actor.shape.yOffset += 400.0f;
        }
    }
}

void EnTite_SetupFlipUpright(EnTite* pthis) {
    pthis->flipState = TEKTITE_UNFLIPPED;
    pthis->actionVar1 = 1000; // value unused here and overwritten in SetupIdle
    //! @bug flying tektite: water sets gravity to 0 so y velocity will never decrease from 13
    pthis->actor.velocity.y = 13.0f;
    Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_TEKU_REVERSE);
    EnTite_SetupAction(pthis, EnTite_FlipUpright);
}

void EnTite_FlipUpright(EnTite* pthis, GlobalContext* globalCtx) {
    Math_SmoothStepToS(&pthis->actor.shape.rot.z, 0, 1, 0xFA0, 0);
    SkelAnime_Update(&pthis->skelAnime);
    //! @bug flying tektite: the following condition is never met and tektite stays stuck in pthis action forever
    if (pthis->actor.bgCheckFlags & 2) {
        func_80033480(globalCtx, &pthis->frontLeftFootPos, 1.0f, 2, 80, 15, 1);
        func_80033480(globalCtx, &pthis->frontRightFootPos, 1.0f, 2, 80, 15, 1);
        func_80033480(globalCtx, &pthis->backRightFootPos, 1.0f, 2, 80, 15, 1);
        func_80033480(globalCtx, &pthis->backLeftFootPos, 1.0f, 2, 80, 15, 1);
        pthis->actor.shape.yOffset = 0.0f;
        pthis->actor.world.pos.y = pthis->actor.floorHeight;
        Audio_PlayActorSound2(&pthis->actor, NA_SE_EN_DODO_M_GND);
        EnTite_SetupIdle(pthis);
    }
}

void EnTite_CheckDamage(Actor* thisx, GlobalContext* globalCtx) {
    EnTite* pthis = (EnTite*)thisx;

    if ((pthis->collider.base.acFlags & AC_HIT) && (pthis->action >= TEKTITE_IDLE)) {
        pthis->collider.base.acFlags &= ~AC_HIT;
        if (thisx->colChkInfo.damageEffect != 0xE) { // Immune to fire magic
            pthis->damageEffect = thisx->colChkInfo.damageEffect;
            Actor_SetDropFlag(thisx, &pthis->collider.elements[0].info, 0);
            // Stun if Tektite hit by nut, boomerang, hookshot, ice arrow or ice magic
            if ((thisx->colChkInfo.damageEffect == 1) || (thisx->colChkInfo.damageEffect == 0xF)) {
                if (pthis->action != TEKTITE_STUNNED) {
                    Actor_SetColorFilter(thisx, 0, 0x78, 0, 0x50);
                    Actor_ApplyDamage(thisx);
                    EnTite_SetupStunned(pthis);
                }
                // Otherwise apply damage and handle death where necessary
            } else {
                if ((thisx->colorFilterTimer == 0) || ((thisx->colorFilterParams & 0x4000) == 0)) {
                    Actor_SetColorFilter(thisx, 0x4000, 0xFF, 0, 8);
                    Actor_ApplyDamage(thisx);
                }
                if (thisx->colChkInfo.health == 0) {
                    EnTite_SetupDeathCry(pthis);
                } else {
                    // Flip tektite back up if it's on its back
                    Audio_PlayActorSound2(thisx, NA_SE_EN_TEKU_DAMAGE);
                    if (pthis->flipState != TEKTITE_FLIPPED) {
                        EnTite_SetupRecoil(pthis);
                    } else {
                        EnTite_SetupFlipUpright(pthis);
                    }
                }
            }
        }
        // If hammer has recently hit the floor and player is close to tektite, flip over
    } else if ((thisx->colChkInfo.health != 0) && (globalCtx->actorCtx.unk_02 != 0) &&
               (thisx->xzDistToPlayer <= 400.0f) && (thisx->bgCheckFlags & 1)) {
        if (pthis->flipState == TEKTITE_FLIPPED) {
            EnTite_SetupFlipUpright(pthis);
        } else if ((pthis->action >= TEKTITE_IDLE) || (pthis->action >= TEKTITE_IDLE)) {
            if (1) {}
            EnTite_SetupFlipOnBack(pthis);
        }
    }
}

void EnTite_Update(Actor* thisx, GlobalContext* globalCtx) {
    EnTite* pthis = (EnTite*)thisx;
    char pad[0x4];
    CollisionPoly* floorPoly;
    WaterBox* waterBox;
    f32 waterSurfaceY;

    EnTite_CheckDamage(thisx, globalCtx);
    // Stay still if hit by immunity damage type pthis frame
    if (thisx->colChkInfo.damageEffect != 0xE) {
        pthis->actionFunc(pthis, globalCtx);
        Actor_MoveForward(thisx);
        Actor_UpdateBgCheckInfo(globalCtx, thisx, 25.0f, 40.0f, 20.0f, pthis->unk_2DC);
        // If on water, snap feet to surface and spawn ripples
        if ((pthis->actor.params == TEKTITE_BLUE) && (thisx->bgCheckFlags & 0x20)) {
            floorPoly = thisx->floorPoly;
            if ((((globalCtx->gameplayFrames % 8) == 0) || (thisx->velocity.y < 0.0f)) &&
                (WaterBox_GetSurfaceImpl(globalCtx, &globalCtx->colCtx, pthis->backRightFootPos.x,
                                         pthis->backRightFootPos.z, &waterSurfaceY, &waterBox)) &&
                (pthis->backRightFootPos.y <= waterSurfaceY)) {
                pthis->backRightFootPos.y = waterSurfaceY;
                EffectSsGRipple_Spawn(globalCtx, &pthis->backRightFootPos, 0, 220, 0);
            }
            if (((((globalCtx->gameplayFrames + 2) % 8) == 0) || (thisx->velocity.y < 0.0f)) &&
                (WaterBox_GetSurfaceImpl(globalCtx, &globalCtx->colCtx, pthis->backLeftFootPos.x,
                                         pthis->backLeftFootPos.z, &waterSurfaceY, &waterBox)) &&
                (pthis->backLeftFootPos.y <= waterSurfaceY)) {
                pthis->backLeftFootPos.y = waterSurfaceY;
                EffectSsGRipple_Spawn(globalCtx, &pthis->backLeftFootPos, 0, 220, 0);
            }
            if (((((globalCtx->gameplayFrames + 4) % 8) == 0) || (thisx->velocity.y < 0.0f)) &&
                (WaterBox_GetSurfaceImpl(globalCtx, &globalCtx->colCtx, pthis->frontLeftFootPos.x,
                                         pthis->frontLeftFootPos.z, &waterSurfaceY, &waterBox)) &&
                (pthis->frontLeftFootPos.y <= waterSurfaceY)) {
                pthis->frontLeftFootPos.y = waterSurfaceY;
                EffectSsGRipple_Spawn(globalCtx, &pthis->frontLeftFootPos, 0, 220, 0);
            }
            if (((((globalCtx->gameplayFrames + 1) % 8) == 0) || (thisx->velocity.y < 0.0f)) &&
                (WaterBox_GetSurfaceImpl(globalCtx, &globalCtx->colCtx, pthis->frontRightFootPos.x,
                                         pthis->frontRightFootPos.z, &waterSurfaceY, &waterBox)) &&
                (pthis->frontRightFootPos.y <= waterSurfaceY)) {
                pthis->frontRightFootPos.y = waterSurfaceY;
                EffectSsGRipple_Spawn(globalCtx, &pthis->frontRightFootPos, 0, 220, 0);
            }
            thisx->floorPoly = floorPoly;
        }

        // If on ground and currently flipped over, set tektite to be fully upside-down
        if (thisx->bgCheckFlags & 3) {
            func_800359B8(thisx, thisx->shape.rot.y, &thisx->shape.rot);
            if (pthis->flipState >= TEKTITE_FLIPPED) {
                thisx->shape.rot.z += 0x7FFF;
            }
            // Otherwise ensure the tektite is rotating back upright
        } else {
            Math_SmoothStepToS(&thisx->shape.rot.x, 0, 1, 1000, 0);
            if (pthis->flipState <= TEKTITE_UNFLIPPED) {
                Math_SmoothStepToS(&thisx->shape.rot.z, 0, 1, 1000, 0);
                if (thisx->shape.yOffset > 0) {
                    thisx->shape.yOffset -= 400.0f;
                }
            }
        }
    }
    thisx->focus.pos = thisx->world.pos;
    thisx->focus.pos.y += 20.0f;

    CollisionCheck_SetAC(globalCtx, &globalCtx->colChkCtx, &pthis->collider.base);
    CollisionCheck_SetOC(globalCtx, &globalCtx->colChkCtx, &pthis->collider.base);
}

void EnTite_PostLimbDraw(GlobalContext* globalCtx, s32 limbIndex, Gfx** limbDList, Vec3s* rot, void* thisx) {
    EnTite* pthis = (EnTite*)thisx;

    switch (limbIndex) {
        case 8:
            Matrix_MultVec3f(&sFootOffset, &pthis->backRightFootPos);
            break;
        case 13:
            Matrix_MultVec3f(&sFootOffset, &pthis->frontRightFootPos);
            break;
        case 18:
            Matrix_MultVec3f(&sFootOffset, &pthis->backLeftFootPos);
            break;
        case 23:
            Matrix_MultVec3f(&sFootOffset, &pthis->frontLeftFootPos);
            break;
    }

    BodyBreak_SetInfo(&pthis->bodyBreak, limbIndex, 0, 24, 24, limbDList, BODYBREAK_OBJECT_DEFAULT);
}

void EnTite_Draw(Actor* thisx, GlobalContext* globalCtx) {
    EnTite* pthis = (EnTite*)thisx;

    OPEN_DISPS(globalCtx->state.gfxCtx, "../z_en_tite.c", 1704);
    func_80093D18(globalCtx->state.gfxCtx);
    Collider_UpdateSpheres(0, &pthis->collider);
    if (pthis->actor.params == TEKTITE_BLUE) {
        gSPSegment(POLY_OPA_DISP++, 0x08, SEGMENTED_TO_VIRTUAL(object_tite_Tex_001300));
        gSPSegment(POLY_OPA_DISP++, 0x09, SEGMENTED_TO_VIRTUAL(object_tite_Tex_001700));
        gSPSegment(POLY_OPA_DISP++, 0x0A, SEGMENTED_TO_VIRTUAL(object_tite_Tex_001900));
    } else {
        gSPSegment(POLY_OPA_DISP++, 0x08, SEGMENTED_TO_VIRTUAL(object_tite_Tex_001B00));
        gSPSegment(POLY_OPA_DISP++, 0x09, SEGMENTED_TO_VIRTUAL(object_tite_Tex_001F00));
        gSPSegment(POLY_OPA_DISP++, 0x0A, SEGMENTED_TO_VIRTUAL(object_tite_Tex_002100));
    }
    SkelAnime_DrawOpa(globalCtx, pthis->skelAnime.skeleton, pthis->skelAnime.jointTable, NULL, EnTite_PostLimbDraw,
                      thisx);
    CLOSE_DISPS(globalCtx->state.gfxCtx, "../z_en_tite.c", 1735);

    if (pthis->spawnIceTimer != 0) {
        // Spawn chunks of ice all over the tektite's body
        thisx->colorFilterTimer++;
        pthis->spawnIceTimer--;
        if ((pthis->spawnIceTimer & 3) == 0) {
            Vec3f iceChunk;
            s32 idx = pthis->spawnIceTimer >> 2;

            iceChunk.x = thisx->world.pos.x + sIceChunks[idx].x;
            iceChunk.y = thisx->world.pos.y + sIceChunks[idx].y;
            iceChunk.z = thisx->world.pos.z + sIceChunks[idx].z;
            EffectSsEnIce_SpawnFlyingVec3f(globalCtx, &pthis->actor, &iceChunk, 150, 150, 150, 250, 235, 245, 255, 1.0f);
        }
    }
}

void EnTite_Reset(Actor* pthisx, GlobalContext* globalCtx) {
    En_Tite_InitVars = {
        ACTOR_EN_TITE,
        ACTORCAT_ENEMY,
        FLAGS,
        OBJECT_TITE,
        sizeof(EnTite),
        (ActorFunc)EnTite_Init,
        (ActorFunc)EnTite_Destroy,
        (ActorFunc)EnTite_Update,
        (ActorFunc)EnTite_Draw,
        (ActorFunc)EnTite_Reset,
    };

    sJntSphInit = {
        {
            COLTYPE_HIT6,
            AT_ON | AT_TYPE_ENEMY,
            AC_ON | AC_TYPE_PLAYER,
            OC1_ON | OC1_TYPE_ALL,
            OC2_TYPE_1,
            COLSHAPE_JNTSPH,
        },
        1,
        sJntSphElementsInit,
    };

    sFootOffset = { 2800.0f, -200.0f, 0.0f };

}
