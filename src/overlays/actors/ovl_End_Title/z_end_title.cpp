#define INTERNAL_SRC_OVERLAYS_ACTORS_OVL_END_TITLE_Z_END_TITLE_C
#include "actor_common.h"
/*
 * File: z_end_title.c
 * Overlay: ovl_End_Title
 * Description: "The End" message
 */

#include "z_end_title.h"
#include "def/sys_matrix.h"
#include "def/z_rcp.h"
#include "def/z_title.h"

#define FLAGS (ACTOR_FLAG_4 | ACTOR_FLAG_5)

void EndTitle_Init(Actor* thisx, GlobalContext* globalCtx);
void EndTitle_Reset(Actor* pthisx, GlobalContext* globalCtx);
void EndTitle_Destroy(Actor* thisx, GlobalContext* globalCtx);
void EndTitle_Update(Actor* thisx, GlobalContext* globalCtx);
void EndTitle_DrawFull(Actor* thisx, GlobalContext* globalCtx);
void EndTitle_DrawNintendoLogo(Actor* thisx, GlobalContext* globalCtx);

ActorInit End_Title_InitVars = {
    ACTOR_END_TITLE,
    ACTORCAT_ITEMACTION,
    FLAGS,
    OBJECT_GAMEPLAY_KEEP,
    sizeof(EndTitle),
    (ActorFunc)EndTitle_Init,
    (ActorFunc)EndTitle_Destroy,
    (ActorFunc)EndTitle_Update,
    (ActorFunc)EndTitle_DrawFull,
    (ActorFunc)EndTitle_Reset,
};

#include "overlays/ovl_End_Title/ovl_End_Title.cpp"

void EndTitle_Init(Actor* thisx, GlobalContext* globalCtx) {
    EndTitle* pthis = (EndTitle*)thisx;

    pthis->endAlpha = 0;
    pthis->tlozAlpha = 0;
    pthis->ootAlpha = 0;
    if (pthis->actor.params == 1) {
        pthis->actor.draw = EndTitle_DrawNintendoLogo;
    }
}

void EndTitle_Destroy(Actor* thisx, GlobalContext* globalCtx) {
}

void EndTitle_Update(Actor* thisx, GlobalContext* globalCtx) {
}

// Used in the castle courtyard
void EndTitle_DrawFull(Actor* thisx, GlobalContext* globalCtx) {
    MtxF* mf;
    EndTitle* pthis = (EndTitle*)thisx;
    const auto& frameCount = globalCtx->csCtx.frames;
    Player* player = GET_PLAYER(globalCtx);

    mf = &player->mf_9E0;

    OPEN_DISPS(globalCtx->state.gfxCtx, "../z_end_title.c", 403);

    // Draw the Triforce on Link's left hand
    func_80093D84(globalCtx->state.gfxCtx);
    Matrix_Mult(mf, MTXMODE_NEW);
    Matrix_Translate(0.0f, 150.0f, 170.0f, MTXMODE_APPLY);
    Matrix_Scale(0.13f, 0.13f, 0.13f, MTXMODE_APPLY);
    Matrix_RotateX(0xBB8 * M_PI / 0x8000, MTXMODE_APPLY);
    Matrix_RotateY(0.0f, MTXMODE_APPLY);
    Matrix_RotateZ(0.0f, MTXMODE_APPLY);
    gSPMatrix(POLY_XLU_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx, "../z_end_title.c", 412), G_MTX_LOAD);
    gSPDisplayList(POLY_XLU_DISP++, sTriforceDL);

    CLOSE_DISPS(globalCtx->state.gfxCtx, "../z_end_title.c", 417);

    OPEN_DISPS(globalCtx->state.gfxCtx, "../z_end_title.c", 419);

    // Draw title cards on the screen
    if ((frameCount > 890) && (pthis->endAlpha < 200)) {
        pthis->endAlpha += 7;
    }
    if ((frameCount > 810) && (pthis->tlozAlpha < 200)) {
        pthis->tlozAlpha += 15;
    }
    if ((frameCount > 850) && (pthis->ootAlpha < 200)) {
        pthis->ootAlpha += 15;
    }

    OVERLAY_DISP = func_80093F34(OVERLAY_DISP);
    gDPSetTextureLUT(OVERLAY_DISP++, G_TT_NONE);
    gDPSetEnvColor(OVERLAY_DISP++, 255, 120, 30, 0);
    gDPSetRenderMode(OVERLAY_DISP++, G_RM_PASS, G_RM_XLU_SURF2);
    gSPClearGeometryMode(OVERLAY_DISP++,
                         G_TEXTURE_ENABLE | G_CULL_BACK | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR);
    gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0,
                      COMBINED, 0, 0, 0, COMBINED);
    gDPSetPrimColor(OVERLAY_DISP++, 0x00, 0x80, 0, 0, 0, pthis->endAlpha);
    gDPLoadTextureTile(OVERLAY_DISP++, sTheEndTex, G_IM_FMT_IA, G_IM_SIZ_8b, 80, 24, 0, 0, 80, 24, 0,
                       G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 0, 0, 0, 0);
    gSPTextureRectangle(OVERLAY_DISP++, 120 << 2, 90 << 2, 200 << 2, 113 << 2, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
    gDPPipeSync(OVERLAY_DISP++);
    gDPSetPrimColor(OVERLAY_DISP++, 0x00, 0x80, 0, 0, 0, pthis->tlozAlpha);
    gDPLoadTextureTile(OVERLAY_DISP++, sTheLegendOfZeldaTex, G_IM_FMT_IA, G_IM_SIZ_8b, 120, 24, 0, 0, 120, 24, 0,
                       G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 0, 0, 0, 0);
    gSPTextureRectangle(OVERLAY_DISP++, 100 << 2, 160 << 2, 220 << 2, 183 << 2, G_TX_RENDERTILE, 0, 0, 1 << 10,
                        1 << 10);
    gDPPipeSync(OVERLAY_DISP++);
    gDPSetPrimColor(OVERLAY_DISP++, 0x00, 0x80, 0, 0, 0, pthis->ootAlpha);
    gDPLoadTextureTile(OVERLAY_DISP++, sOcarinaOfTimeTex, G_IM_FMT_IA, G_IM_SIZ_8b, 112, 16, 0, 0, 112, 16, 0,
                       G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 0, 0, 0, 0);
    gSPTextureRectangle(OVERLAY_DISP++, 104 << 2, 177 << 2, 216 << 2, 192 << 2, G_TX_RENDERTILE, 0, 0, 1 << 10,
                        1 << 10);

    CLOSE_DISPS(globalCtx->state.gfxCtx, "../z_end_title.c", 515);
}

// Used in the Temple of Time
void EndTitle_DrawNintendoLogo(Actor* thisx, GlobalContext* globalCtx) {
    EndTitle* pthis = (EndTitle*)thisx;
    s32 pad;
    const auto& frames = globalCtx->csCtx.frames;

    if ((frames >= 1101) && (pthis->endAlpha < 255)) {
        pthis->endAlpha += 3;
    }

    OPEN_DISPS(globalCtx->state.gfxCtx, "../z_end_title.c", 594);

    OVERLAY_DISP = func_80093F34(OVERLAY_DISP);
    gDPSetPrimColor(OVERLAY_DISP++, 0, 0x80, 0, 0, 0, pthis->endAlpha);
    gSPDisplayList(OVERLAY_DISP++, sPresentedByNintendoDL);

    CLOSE_DISPS(globalCtx->state.gfxCtx, "../z_end_title.c", 600);
}

void EndTitle_Reset(Actor* pthisx, GlobalContext* globalCtx) {
    End_Title_InitVars = {
        ACTOR_END_TITLE,
        ACTORCAT_ITEMACTION,
        FLAGS,
        OBJECT_GAMEPLAY_KEEP,
        sizeof(EndTitle),
        (ActorFunc)EndTitle_Init,
        (ActorFunc)EndTitle_Destroy,
        (ActorFunc)EndTitle_Update,
        (ActorFunc)EndTitle_DrawFull,
        (ActorFunc)EndTitle_Reset,
    };

}
