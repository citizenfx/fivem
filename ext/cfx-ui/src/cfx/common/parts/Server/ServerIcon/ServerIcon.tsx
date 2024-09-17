import { Indicator, clsx } from '@cfx-dev/ui-components';

import { getServerIconPlaceholder, getServerIconURL } from 'cfx/common/services/servers/icon';
import { IServerView, ServerViewDetailsLevel } from 'cfx/common/services/servers/types';

import s from './ServerIcon.module.scss';

type TypeProps = { type: 'list'; loading?: boolean } | { type: 'details'; size?: 'small' | 'normal' };

export type ServerIconProps = TypeProps & {
  server: IServerView | null | undefined;
  glow?: boolean;
  className?: string;
};
export function ServerIcon(props: ServerIconProps) {
  const {
    server,
    type,
    className,
    glow = false,
  } = props;

  const isList = type === 'list';
  const isDetails = type === 'details';

  const iconURL = useServerIconURL(server);

  const rootClassName = clsx(
    s.root,
    className,
    s[`type-${type}`],
    // eslint-disable-next-line react/destructuring-assignment
    isDetails && s[`size-${props.size || 'normal'}`],
    {
      [s.glow]: glow,
    },
  );

  return (
    <div className={rootClassName}>
      {glow && (
        <div className={s.blur}>
          <img src={iconURL} alt="" />
        </div>
      )}

      <img alt={server?.id} src={iconURL} className={s.icon} />

      {/* eslint-disable-next-line react/destructuring-assignment */}
      {isList && !!props.loading && (
        <div className={s.loader}>
          <Indicator />
        </div>
      )}
    </div>
  );
}

const cache: Record<string, { level: ServerViewDetailsLevel; url: string }> = {};
let fallbackIconURL: string | null = null;

function useServerIconURL(server: IServerView | null | undefined): string {
  if (!server) {
    if (!fallbackIconURL) {
      fallbackIconURL = getServerIconPlaceholder('__FALLBACK__');
    }

    return fallbackIconURL;
  }

  const cacheKey = server.id;

  const cachedEntry = cache[cacheKey];

  if (!cachedEntry || cachedEntry.level !== server.detailsLevel) {
    cache[cacheKey] = {
      level: server.detailsLevel,
      url: getServerIconURL(server),
    };
  }

  return cache[cacheKey].url;
}
