import * as md5 from 'js-md5';

export function getCustomInGameBackdropPath(backdrop: string | undefined): string {
  if (!backdrop) {
    return '';
  }

  return `https://nui-backdrop/user.png?${md5(backdrop ?? '')}`;
}
