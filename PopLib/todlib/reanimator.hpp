#ifndef __REANIMATION_HPP__
#define __REANIMATION_HPP__

#include "dataarray.hpp"
#include "filtereffect.hpp"
#include "math/matrix.hpp"
using namespace std;
using namespace PopLib;

class Reanimation;
class ReanimAtlas;
class AttacherInfo;
class AttachEffect;
class TodTriangleGroup;
class TodParticleSystem;
class ReanimatorTransform;
class ReanimatorDefinition;
namespace PopLib
{
    class Font;
    class Image;
    class Graphics;
    class MemoryImage;
};

// ######################################################################################################################################################
// ############################################################### 以下为动画定义相关内容 ###############################################################
// ######################################################################################################################################################

constexpr const float DEFAULT_FIELD_PLACEHOLDER = -10000.0f;
constexpr const double SECONDS_PER_UPDATE = 0.01f;

#ifndef COMPILING_PVZ
enum ReanimationType
{
    REANIM_NONE = -1,
    REANIM_EXAMPLE = 0,
    NUM_REANIMS
};
#else
enum ReanimationType
{
    REANIM_NONE = -1,
    REANIM_LOADBAR_SPROUT,
    REANIM_LOADBAR_ZOMBIEHEAD,
    REANIM_SODROLL,
    REANIM_FINAL_WAVE,
    REANIM_PEASHOOTER,
    REANIM_WALLNUT,
    REANIM_LILYPAD,
    REANIM_SUNFLOWER,
    REANIM_LAWNMOWER,
    REANIM_READYSETPLANT,
    REANIM_CHERRYBOMB,
    REANIM_SQUASH,
    REANIM_DOOMSHROOM,
    REANIM_SNOWPEA,
    REANIM_REPEATER,
    REANIM_SUNSHROOM,
    REANIM_TALLNUT,
    REANIM_FUMESHROOM,
    REANIM_PUFFSHROOM,
    REANIM_HYPNOSHROOM,
    REANIM_CHOMPER,
    REANIM_ZOMBIE,
    REANIM_SUN,
    REANIM_POTATOMINE,
    REANIM_SPIKEWEED,
    REANIM_SPIKEROCK,
    REANIM_THREEPEATER,
    REANIM_MARIGOLD,
    REANIM_ICESHROOM,
    REANIM_ZOMBIE_FOOTBALL,
    REANIM_ZOMBIE_NEWSPAPER,
    REANIM_ZOMBIE_ZAMBONI,
    REANIM_SPLASH,
    REANIM_JALAPENO,
    REANIM_JALAPENO_FIRE,
    REANIM_COIN_SILVER,
    REANIM_ZOMBIE_CHARRED,
    REANIM_ZOMBIE_CHARRED_IMP,
    REANIM_ZOMBIE_CHARRED_DIGGER,
    REANIM_ZOMBIE_CHARRED_ZAMBONI,
    REANIM_ZOMBIE_CHARRED_CATAPULT,
    REANIM_ZOMBIE_CHARRED_GARGANTUAR,
    REANIM_SCRAREYSHROOM,
    REANIM_PUMPKIN,
    REANIM_PLANTERN,
    REANIM_TORCHWOOD,
    REANIM_SPLITPEA,
    REANIM_SEASHROOM,
    REANIM_BLOVER,
    REANIM_FLOWER_POT,
    REANIM_CACTUS,
    REANIM_DANCER,
    REANIM_TANGLEKELP,
    REANIM_STARFRUIT,
    REANIM_POLEVAULTER,
    REANIM_BALLOON,
    REANIM_GARGANTUAR,
    REANIM_IMP,
    REANIM_DIGGER,
    REANIM_DIGGER_DIRT,
    REANIM_ZOMBIE_DOLPHINRIDER,
    REANIM_POGO,
    REANIM_BACKUP_DANCER,
    REANIM_BOBSLED,
    REANIM_JACKINTHEBOX,
    REANIM_SNORKEL,
    REANIM_BUNGEE,
    REANIM_CATAPULT,
    REANIM_LADDER,
    REANIM_PUFF,
    REANIM_SLEEPING,
    REANIM_GRAVE_BUSTER,
    REANIM_ZOMBIES_WON,
    REANIM_MAGNETSHROOM,
    REANIM_BOSS,
    REANIM_CABBAGEPULT,
    REANIM_KERNELPULT,
    REANIM_MELONPULT,
    REANIM_COFFEEBEAN,
    REANIM_UMBRELLALEAF,
    REANIM_GATLINGPEA,
    REANIM_CATTAIL,
    REANIM_GLOOMSHROOM,
    REANIM_BOSS_ICEBALL,
    REANIM_BOSS_FIREBALL,
    REANIM_COBCANNON,
    REANIM_GARLIC,
    REANIM_GOLD_MAGNET,
    REANIM_WINTER_MELON,
    REANIM_TWIN_SUNFLOWER,
    REANIM_POOL_CLEANER,
    REANIM_ROOF_CLEANER,
    REANIM_FIRE_PEA,
    REANIM_IMITATER,
    REANIM_YETI,
    REANIM_BOSS_DRIVER,
    REANIM_LAWN_MOWERED_ZOMBIE,
    REANIM_CRAZY_DAVE,
    REANIM_TEXT_FADE_ON,
    REANIM_HAMMER,
    REANIM_SLOT_MACHINE_HANDLE,
    REANIM_CREDITS_FOOTBALL,
    REANIM_CREDITS_JACKBOX,
    REANIM_SELECTOR_SCREEN,
    REANIM_PORTAL_CIRCLE,
    REANIM_PORTAL_SQUARE,
    REANIM_ZENGARDEN_SPROUT,
    REANIM_ZENGARDEN_WATERINGCAN,
    REANIM_ZENGARDEN_FERTILIZER,
    REANIM_ZENGARDEN_BUGSPRAY,
    REANIM_ZENGARDEN_PHONOGRAPH,
    REANIM_DIAMOND,
    REANIM_ZOMBIE_HAND,
    REANIM_STINKY,
    REANIM_RAKE,
    REANIM_RAIN_CIRCLE,
    REANIM_RAIN_SPLASH,
    REANIM_ZOMBIE_SURPRISE,
    REANIM_COIN_GOLD,
    REANIM_TREEOFWISDOM,
    REANIM_TREEOFWISDOM_CLOUDS,
    REANIM_TREEOFWISDOM_TREEFOOD,
    REANIM_CREDITS_MAIN,
    REANIM_CREDITS_MAIN2,
    REANIM_CREDITS_MAIN3,
    REANIM_ZOMBIE_CREDITS_DANCE,
    REANIM_CREDITS_STAGE,
    REANIM_CREDITS_BIGBRAIN,
    REANIM_CREDITS_FLOWER_PETALS,
    REANIM_CREDITS_INFANTRY,
    REANIM_CREDITS_THROAT,
    REANIM_CREDITS_CRAZYDAVE,
    REANIM_CREDITS_BOSSDANCE,
    REANIM_ZOMBIE_CREDITS_SCREEN_DOOR,
    REANIM_ZOMBIE_CREDITS_CONEHEAD,
    REANIM_CREDITS_ZOMBIEARMY1,
    REANIM_CREDITS_ZOMBIEARMY2,
    REANIM_CREDITS_TOMBSTONES,
    REANIM_CREDITS_SOLARPOWER,
    REANIM_CREDITS_ANYHOUR,
    REANIM_CREDITS_WEARETHEUNDEAD,
    REANIM_CREDITS_DISCOLIGHTS,
    REANIM_FLAG,
    NUM_REANIMS
};
#endif

enum ReanimFlags
{
    REANIM_NO_ATLAS,
    REANIM_FAST_DRAW_IN_SW_MODE
};

class ReanimatorTrack
{
public:
    const char*                     mName;                          //+0x0：轨道名称
    ReanimatorTransform*            mTransforms;                    //+0x4：每一帧的动画变换的数组
    int                             mTransformCount;                //+0x8：动画变换数量，即帧数量
    
public:
    ReanimatorTrack() : mName(""), mTransforms(nullptr), mTransformCount(0) { }
};

// ====================================================================================================
// ★ 【动画器定义】
// ----------------------------------------------------------------------------------------------------
// 用于描述一种动画类型与该动画的数据文件的文件名及标志之间的对应关系。
// ====================================================================================================
class ReanimatorDefinition
{
public:
    ReanimatorTrack*                mTracks;
    int                             mTrackCount;
    float                           mFPS;
    ReanimAtlas*                    mReanimAtlas;

public:
    ReanimatorDefinition() : mTracks(nullptr), mTrackCount(0), mFPS(12.0f), mReanimAtlas(nullptr) { }
};
extern int gReanimatorDefCount;                     //[0x6A9EE4]
extern ReanimatorDefinition* gReanimatorDefArray;   //[0x6A9EE8]

// ====================================================================================================
// ★ 【动画参数】
// ----------------------------------------------------------------------------------------------------
// 用于描述一种动画类型与该动画的数据文件的文件名及标志之间的对应关系。
// ====================================================================================================
class ReanimationParams
{
public:
    ReanimationType                 mReanimationType;
    const char*                     mReanimFileName;
    int                             mReanimParamFlags;
};
extern int gReanimationParamArraySize;              //[0x6A9EEC]
extern ReanimationParams* gReanimationParamArray;   //[0x6A9EF0]

/*inline*/ void                     ReanimationFillInMissingData(float& thePrev, float& theValue);
/*inline*/ void                     ReanimationFillInMissingData(void*& thePrev, void*& theValue);
bool                                ReanimationLoadDefinition(const PopString& theFileName, ReanimatorDefinition* theDefinition);
void                                ReanimationFreeDefinition(ReanimatorDefinition* theDefinition);
void _cdecl	                        ReanimatorEnsureDefinitionLoaded(ReanimationType theReanimType, bool theIsPreloading);
void                                ReanimatorLoadDefinitions(ReanimationParams* theReanimationParamArray, int theReanimationParamArraySize);
void                                ReanimatorFreeDefinitions();

// ######################################################################################################################################################
// ############################################################## 以下正式开始动画相关声明 ##############################################################
// ######################################################################################################################################################

enum
{
    RENDER_GROUP_HIDDEN = -1,
    RENDER_GROUP_NORMAL = 0
};

constexpr const int NO_BASE_POSE = -2;

class ReanimationHolder
{
public:
    DataArray<Reanimation>          mReanimations;

public:
    ReanimationHolder() { ; }
    ~ReanimationHolder();

    void                            InitializeHolder();
    void                            DisposeHolder();
    Reanimation*                    AllocReanimation(float theX, float theY, int theRenderOrder, ReanimationType theReanimationType);
};

// ====================================================================================================
// ★ 【动画器时间】
// ----------------------------------------------------------------------------------------------------
// 用于描述动画当前正在播放的时间位置。
// ====================================================================================================
class ReanimatorFrameTime
{
public:
    float                           mFraction;                      //+0x0：两帧之间已经过的比例
    int                             mAnimFrameBeforeInt;            //+0x4：前一个整数帧
    int                             mAnimFrameAfterInt;             //+0x8：后一个整数帧
};

class ReanimatorTransform
{
public:
    float                           mTransX;
    float                           mTransY;
    float                           mSkewX;
    float                           mSkewY;
    float                           mScaleX;
    float                           mScaleY;
    float                           mFrame;
    float                           mAlpha;
    Image*                          mImage;
    Font*                           mFont;
    const char*                     mText;

public:
    ReanimatorTransform();
};

class ReanimatorTrackInstance
{
public:
    int                             mBlendCounter;                  //+0x0
    int                             mBlendTime;                     //+0x4
    ReanimatorTransform             mBlendTransform;                //+0x8
    float                           mShakeOverride;                 //+0x34
    float                           mShakeX;                        //+0x38
    float                           mShakeY;                        //+0x3C
    AttachmentID                    mAttachmentID;                  //+0x40
    Image*                          mImageOverride;                 //+0x44
    int                             mRenderGroup;                   //+0x48
    Color                           mTrackColor;                    //+0x4C
    bool                            mIgnoreClipRect;                //+0x5C
    bool                            mTruncateDisappearingFrames;    //+0x5D
    bool                            mIgnoreColorOverride;           //+0x5E
    bool                            mIgnoreExtraAdditiveColor;      //+0x5F

public:
    ReanimatorTrackInstance();
};

class Reanimation
{
public:
    ReanimationType                 mReanimationType;
    float                           mAnimTime;
    float                           mAnimRate;
    ReanimatorDefinition*           mDefinition;
    ReanimLoopType                  mLoopType;
    bool                            mDead;
    int                             mFrameStart;
    int                             mFrameCount;
    int                             mFrameBasePose;
    Transform2D                 mOverlayMatrix;
    Color                           mColorOverride;
    ReanimatorTrackInstance*        mTrackInstances;
    int                             mLoopCount;
    ReanimationHolder*              mReanimationHolder;
    bool                            mIsAttachment;
    int                             mRenderOrder;
    Color                           mExtraAdditiveColor;
    bool                            mEnableExtraAdditiveDraw;
    Color                           mExtraOverlayColor;
    bool                            mEnableExtraOverlayDraw;
    float                           mLastFrameTime;
    FilterEffect                    mFilterEffect;

public:
    Reanimation();
    ~Reanimation();

    void                            ReanimationInitialize(float theX, float theY, ReanimatorDefinition* theDefinition);
    /*inline*/ void                 ReanimationInitializeType(float theX, float theY, ReanimationType theReanimType);
    void                            ReanimationDie();
    void                            Update();
    /*inline*/ void                 Draw(Graphics* g);
    void                            DrawRenderGroup(Graphics* g, int theRenderGroup);
    bool                            DrawTrack(Graphics* g, int theTrackIndex, int theRenderGroup, TodTriangleGroup* theTriangleGroup);
    void                            GetCurrentTransform(int theTrackIndex, ReanimatorTransform* theTransformCurrent);
    void                            GetTransformAtTime(int theTrackIndex, ReanimatorTransform* theTransform, ReanimatorFrameTime* theFrameTime);
    void                            GetFrameTime(ReanimatorFrameTime* theFrameTime);
    int                             FindTrackIndex(const char* theTrackName);
    void                            AttachToAnotherReanimation(Reanimation* theAttachReanim, const char* theTrackName);
    void                            GetAttachmentOverlayMatrix(int theTrackIndex, Transform2D& theOverlayMatrix);
    /*inline*/ void                 SetFramesForLayer(const char* theTrackName);
    static void                     MatrixFromTransform(const ReanimatorTransform& theTransform, Matrix3& theMatrix);
    bool                            TrackExists(const char* theTrackName);
    void                            StartBlend(int theBlendTime);
    /*inline*/ void                 SetShakeOverride(const char* theTrackName, float theShakeAmount);
    /*inline*/ void                 SetPosition(float theX, float theY);
    /*inline*/ void                 OverrideScale(float theScaleX, float theScaleY);
    float                           GetTrackVelocity(const char* theTrackName);
    /*inline*/ void                 SetImageOverride(const char* theTrackName, Image* theImage);
    /*inline*/ Image*               GetImageOverride(const char* theTrackName);
    void                            ShowOnlyTrack(const char* theTrackName);
    void                            GetTrackMatrix(int theTrackIndex, Transform2D& theMatrix);
    void                            AssignRenderGroupToTrack(const char* theTrackName, int theRenderGroup);
    void                            AssignRenderGroupToPrefix(const char* theTrackName, int theRenderGroup);
    void                            PropogateColorToAttachments();
    bool                            ShouldTriggerTimedEvent(float theEventTime);
    void                            TodTriangleGroupDraw(Graphics* g, TodTriangleGroup* theTriangleGroup) { ; }
    Image*                          GetCurrentTrackImage(const char* theTrackName);
    AttachEffect*                   AttachParticleToTrack(const char* theTrackName, TodParticleSystem* theParticleSystem, float thePosX, float thePosY);
    void                            GetTrackBasePoseMatrix(int theTrackIndex, Transform2D& theBasePosMatrix);
    bool                            IsTrackShowing(const char* theTrackName);
    /*inline*/ void                 SetTruncateDisappearingFrames(const char* theTrackName = nullptr, bool theTruncateDisappearingFrames = false);
    /*inline*/ void                 PlayReanim(const char* theTrackName, ReanimLoopType theLoopType, int theBlendTime, float theAnimRate);
    void                            ReanimationDelete();
    ReanimatorTrackInstance*        GetTrackInstanceByName(const char* theTrackName);
    void                            GetFramesForLayer(const char* theTrackName, int& theFrameStart, int& theFrameCount);
    void                            UpdateAttacherTrack(int theTrackIndex);
    static void                     ParseAttacherTrack(const ReanimatorTransform& theTransform, AttacherInfo& theAttacherInfo);
    void                            AttacherSynchWalkSpeed(int theTrackIndex, Reanimation* theAttachReanim, AttacherInfo& theAttacherInfo);
    /*inline*/ bool                 IsAnimPlaying(const char* theTrackName);
    void                            SetBasePoseFromAnim(const char* theTrackName);
    void                            ReanimBltMatrix(Graphics* g, Image* theImage, Matrix3& theTransform, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect& theSrcRect);
    Reanimation*                    FindSubReanim(ReanimationType theReanimType);
};

void                                ReanimationCreateAtlas(ReanimatorDefinition* theDefinition, ReanimationType theReanimationType);
void                                ReanimationPreload(ReanimationType theReanimationType);
void                                BlendTransform(ReanimatorTransform* theResult, const ReanimatorTransform& theTransform1, const ReanimatorTransform& theTransform2, float theBlendFactor);

#endif

