import { playSfx, Sfx } from "cfx/apps/mpMenu/utils/sfx";
import { $L } from "cfx/common/services/intl/l10n";
import { useServersService } from "cfx/common/services/servers/servers.service";
import { IServerView } from "cfx/common/services/servers/types";
import { Button, ButtonProps } from "cfx/ui/Button/Button";
import { Icons } from "cfx/ui/Icons";
import { Title } from "cfx/ui/Title/Title";
import { stopPropagation } from "cfx/utils/domEvents";
import { observer } from "mobx-react-lite";

export interface ServerFavoriteButtonProps {
  server: IServerView,

  size?: ButtonProps['size'],
  theme?: ButtonProps['theme'],
}

export const ServerFavoriteButton = observer(function ServerFavoriteButton(props: ServerFavoriteButtonProps) {
  const {
    server,
    size = 'large',
    theme = 'transparent',
  } = props;

  const ServersService = useServersService();

  const favoriteServersList = ServersService.getFavoriteList();
  if (!favoriteServersList) {
    return null;
  }

  const isInFavoriteServersList = favoriteServersList.isIn(server.id);

  const handleClick = () => {
    if (__CFXUI_USE_SOUNDS__) {
      if (isInFavoriteServersList) {
        playSfx(Sfx.Click4);
      } else {
        playSfx(Sfx.Click2);
      }
    }

    favoriteServersList.toggleIn(server.id);
  };

  const title = isInFavoriteServersList
    ? $L('#ServerDetail_DelFavorite')
    : $L('#ServerDetail_AddFavorite');

  return (
    <Title title={title}>
      <Button
        size={size}
        theme={theme}
        icon={
          isInFavoriteServersList
            ? Icons.favoriteActive
            : Icons.favoriteInactive
        }
        onClick={stopPropagation(handleClick)}
      />
    </Title>
  );
});
