import { useServersService } from "cfx/common/services/servers/servers.service";
import { IServerView } from "cfx/common/services/servers/types";
import { Indicator } from "cfx/ui/Indicator/Indicator";
import { clsx } from "cfx/utils/clsx";
import s from './ServerIcon.module.scss';

type TypeProps =
  | { type: 'list', loading?: boolean }
  | { type: 'details', size?: 'small' | 'normal' }

export type ServerIconProps = TypeProps & {
  server: IServerView,
  glow?: boolean,
  className?: string,
}
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
    isDetails && s[`size-${props.size || 'normal'}`],
    {
      [s.glow]: glow,
    },
  );

  return (
    <div className={rootClassName}>
      {glow && (
        <div className={s.blur}>
          <img src={iconURL} />
        </div>
      )}

      <img
        alt={server.id}
        src={iconURL}
        className={s.icon}
      />

      {(isList && !!props.loading) && (
        <div className={s.loader}>
          <Indicator />
        </div>
      )}
    </div>
  );
}

const cache: Record<string, string> = {};

function useServerIconURL(server: IServerView): string {
  const ServersService = useServersService();

  const cacheKey = `${server.id}::${server.detailsLevel}`;

  if (!cache[cacheKey]) {
    let iconURL = '';

    if ('iconVersion' in server) {
      iconURL = ServersService.getServerIconURL(server.id);
    }
    else if (server.thumbnailIconUri) {
      iconURL = server.thumbnailIconUri;
    }
    else {
      iconURL = ServersService.getServerIconURL(server.id);
    }

    cache[cacheKey] = iconURL;
  }

  return cache[cacheKey];
}
