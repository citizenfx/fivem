export enum AuthFormMode {
  Initial,
  ExternalAuthInitialized,
  ExternalAuthProcessing,
  Authenticated,
}

export const authFormModeToAnalyticsLabel: Record<AuthFormMode, string> = {
  [AuthFormMode.Initial]: 'LoginScreen',
  [AuthFormMode.ExternalAuthInitialized]: 'ExternalAuthScreen',
  [AuthFormMode.ExternalAuthProcessing]: 'ExternalAuthScreen',
  [AuthFormMode.Authenticated]: 'AuthenticatedScreen',
};
