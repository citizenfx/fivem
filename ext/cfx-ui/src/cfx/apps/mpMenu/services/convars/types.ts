export type KnownConvars =
  | 'ui_extNickname'
  | 'ui_blurPerfMode'
  | 'ui_disableMusicTheme'
  | 'nui_useInProcessGpu'
  | 'game_showStreamingProgress'
  | 'game_useAudioFrameLimiter'
  | 'cam_enableHandbrakeCamera'
  | 'cam_disableCameraShake'
  | 'ui_customBrandingEmoji'
  | 'ui_premium'
  | 'ui_updateChannel'
  | 'ui_customBackdrop'
  | 'nui_useFixedSize';

export type IConvar = string | KnownConvars;
