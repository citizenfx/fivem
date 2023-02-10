import { hasConnectEndpoints, isServerEOL } from "cfx/base/serverUtils";
import { useServiceOptional } from "cfx/base/servicesContainer";
import { $L } from "cfx/common/services/intl/l10n";
import { IServersConnectService } from "cfx/common/services/servers/serversConnect.service";
import { IServerView } from "cfx/common/services/servers/types";
import { noop } from "cfx/utils/functional";
import { playSfx, Sfx } from "cfx/apps/mpMenu/utils/sfx";
import { observer } from "mobx-react-lite";
import { Title } from "cfx/ui/Title/Title";
import { Button, ButtonProps } from "cfx/ui/Button/Button";
import { Select } from "cfx/ui/Select/Select";
import { ReactNode } from "react";
import { stopPropagation } from "cfx/utils/domEvents";
import { isServerOffline } from "cfx/common/services/servers/helpers";
import { CurrentGameBuild, CurrentGamePureLevel } from "cfx/base/gameRuntime";

export interface ServerConnectButtonProps {
  server: IServerView,
  serverOtherInstances?: IServerView[] | null,

  size?: ButtonProps['size'],
  theme?: ButtonProps['theme'],
}

export const ServerConnectButton = observer(function ServerConnectButton(props: ServerConnectButtonProps) {
  const {
    server,
    serverOtherInstances,
    size = 'large',
    theme = "primary",
  } = props;

  const ServersConnectService = useServiceOptional(IServersConnectService);

  let title: ReactNode;
  let canConnect = hasConnectEndpoints(server);

  if (!server.historicalAddress) {
    switch (true) {
      case isServerEOL(server): {
        title = $L('#ServerDetail_EOLDisable');
        canConnect = false;
        break;
      }

      case !!server.private: {
        title = $L('#ServerDetail_PrivateDisable');
        break;
      }

      case isServerOffline(server): {
        title = $L('#ServerDetail_OfflineDisable');
        break;
      }
    }

    if (canConnect) {
      if (server.enforceGameBuild || server.pureLevel) {
        const hasKnownGameBuild = CurrentGameBuild !== '-1';
        const hasKnownGamePureLevel = CurrentGamePureLevel !== '-1';

        const shouldSwitchGameBuild = hasKnownGameBuild && server.enforceGameBuild && CurrentGameBuild !== server.enforceGameBuild;
        const shouldSwitchPureLevel = hasKnownGamePureLevel && server.pureLevel && CurrentGamePureLevel !== server.pureLevel;

        if (shouldSwitchGameBuild && shouldSwitchPureLevel) {
          title = $L('#DirectConnect_SwitchBuildPureLevelAndConnect');
        } else if (shouldSwitchGameBuild) {
          title = $L('#DirectConnect_SwitchBuildAndConnect');
        } else if (shouldSwitchPureLevel) {
          title = $L('#DirectConnect_SwitchPureLevelAndConnect');
        }
      }
    }
  }

  const disabled = !ServersConnectService?.canConnect || !canConnect;


  const handleClick = ServersConnectService
    ? () => {
      if (__CFXUI_USE_SOUNDS__) {
        playSfx(Sfx.Connect);
      }

      ServersConnectService.connectTo(server);
    }
    : noop;

  let action = (
    <Button
      size={size}
      theme={theme}
      disabled={disabled}
      text={$L('#DirectConnect_Connect')}
      onClick={stopPropagation(handleClick)}
    />
  );


  if (serverOtherInstances && serverOtherInstances.length > 0) {
    let options: { label: string, value: string }[] = [
      { label: "Connect", value: "" },
      { label: (server.instanceName || server.localeCountry), value: server.id },
    ];

    serverOtherInstances.forEach(x => {
      options.push({ label: (x.instanceName || x.localeCountry), value: x.id });
    });


    const handleChange = ServersConnectService
      ? (id: string) => {
        if (__CFXUI_USE_SOUNDS__) {
          playSfx(Sfx.Connect);
        }

        if (id == "") {
          return;
        }

        if (server.id == id) {
          ServersConnectService.connectTo(server);
          return;
        }

        for (const serverOtherInstance of serverOtherInstances) {
          if (serverOtherInstance.id == id) {
            ServersConnectService.connectTo(serverOtherInstance);
            return;
          }
        }
      }
      : noop;

    action = (
      <Select
        value={""}
        options={options}
        onChange={handleChange}
        size={size == "small" ? "small" : "normal"}
      />
    );
  }

  return (
    <Title fixedOn="bottom-left" title={title}>
      {action}
    </Title>
  );
});
