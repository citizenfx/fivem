import { BackdropPortal, clsx } from '@cfx-dev/ui-components';

import { IServerView } from 'cfx/common/services/servers/types';

import s from './ServerBackdropBanner.module.scss';

export interface ServerBackdropBannerProps {
  server: IServerView;

  offsetY?: string;
  animated?: boolean;
}

export function ServerBackdropBanner(props: ServerBackdropBannerProps) {
  const {
    server,
    offsetY = '0px',
    animated = false,
  } = props;

  if (!server.bannerDetail) {
    return null;
  }

  const rootClassName = clsx(s.root, {
    [s.animated]: animated,
  });

  return (
    <BackdropPortal>
      <div
        className={rootClassName}
        style={
          {
            '--banner': `url(${server.bannerDetail})`,
            transform: `translateY(${offsetY})`,
          } as any
        }
      />
    </BackdropPortal>
  );
}
