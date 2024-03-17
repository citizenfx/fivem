export type KnownConvars =
  | 'ui_extNickname'
  | 'ui_blurPerfMode'
  | 'ui_disableMusicTheme'
  | 'ui_customBackdrop'
  | 'ui_updateChannel'
  | 'nui_useInProcessGpu'
  | 'game_showStreamingProgress'
  | 'game_useAudioFrameLimiter'
  | 'ui_muteOnFocusLoss'
  | 'cam_enableHandbrakeCamera'
  | 'cam_disableCameraShake'
  | 'nui_useFixedSize'
  | 'voice_enableNoiseSuppression'
  | 'ui_customBrandingEmoji'
  | 'ui_premium'

export type IConvar = string | KnownConvars;
