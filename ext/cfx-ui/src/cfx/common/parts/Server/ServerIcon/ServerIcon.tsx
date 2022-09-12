import { useService } from "cfx/base/servicesContainer";
import { IServersService } from "cfx/common/services/servers/servers.service";
import { IServerView } from "cfx/common/services/servers/types";
import { Indicator } from "cfx/ui/Indicator/Indicator";
import { clsx } from "cfx/utils/clsx";
import s from './ServerIcon.module.scss';

type TypeProps =
  | { type: 'list', loading?: boolean }
  | { type: 'details', size?: 'small' | 'normal' }

export type ServerIconProps = TypeProps & {
  server: string | IServerView,
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

  const serverId = typeof server === 'string'
    ? server
    : server.id;

  const iconURL = useService(IServersService).getServerIconURL(serverId);

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
        alt={serverId}
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
