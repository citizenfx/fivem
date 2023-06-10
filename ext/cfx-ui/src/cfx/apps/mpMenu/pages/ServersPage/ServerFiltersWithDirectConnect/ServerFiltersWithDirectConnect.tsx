import React from "react";
import { ServerFilters } from "cfx/common/parts/Server/ServerFilters/ServerFilters";
import { ServerListConfigController } from "cfx/common/services/servers/lists/ServerListConfigController";
import { observer } from "mobx-react-lite";
import { useWindowResize } from "cfx/utils/hooks";
import { ui } from "cfx/ui/ui";
import { ServerFiltersWithDirectConnectController, userServerFiltersWithDirectConnectController } from "./ServerFiltersWithDirectConnectController";
import { Indicator } from "cfx/ui/Indicator/Indicator";
import { TitleOutlet } from "cfx/ui/outlets";
import { Shroud } from "cfx/ui/Shroud/Shroud";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { TextBlock } from "cfx/ui/Text/Text";
import { Button } from "cfx/ui/Button/Button";
import { ServerTitle } from "cfx/common/parts/Server/ServerTitle/ServerTitle";
import { ServerTileItem } from "cfx/common/parts/Server/ServerTileItem/ServerTileItem";
import { $L } from "cfx/common/services/intl/l10n";
import s from './ServerFiltersWithDirectConnect.module.scss';

export interface ServerFiltersWithDirectConnectProps {
  config: ServerListConfigController,
}
export const ServerFiltersWithDirectConnect = observer(function ServerFiltersWithDirectConnect(props: ServerFiltersWithDirectConnectProps) {
  const {
    config,
  } = props;

  const inputRef = React.useRef<HTMLElement>(null);
  const controller = userServerFiltersWithDirectConnectController();
  {
    controller.config = config;
  }

  React.useEffect(() => {
    controller.setSearchTerms(config.searchTextParsed);
  }, [controller, config.searchText]);

  const showDirectConnect = controller.inputActive && !!config.searchTextParsed.length && config.searchTextParsed[0]?.type === 'address';

  return (
    <>
      <ServerFilters
        config={config}
        inputRef={inputRef}
        onInputActive={controller.setInputActive}
        onInputKeyDown={controller.handleInputKeyDown}
      />

      {showDirectConnect && (
        <>
          <DirectConnect
            config={config}
            inputRef={inputRef}
            controller={controller}
          />

          <Shroud forRef={inputRef} />
        </>
      )}
    </>
  );
});

interface DirectConnectProps {
  config: ServerListConfigController,
  inputRef: React.RefObject<HTMLElement>,
  controller: ServerFiltersWithDirectConnectController,
}

const DirectConnect = observer(function DirectConnect(props: DirectConnectProps) {
  const {
    config,
    inputRef,
    controller,
  } = props;

  const pos = useDirectConnectPos(inputRef);
  const server = controller.server;

  const label = getDirectConnectLabel(config, controller);

  const rootStyle: any = {
    '--x': ui.px(pos[0]),
    '--y': ui.px(pos[1] + pos[3]),
    '--w': ui.px(pos[2]),
  };

  return (
    <TitleOutlet>
      <div className={s.root} style={rootStyle}>
        <Flex vertical>
          <Flex repell centered>
            <TextBlock opacity="75">
              {label}
            </TextBlock>

            <Button
              size="small"
              theme="primary"
              text={$L('#DirectConnect2_PressEnterToConnect')}
              disabled={!server || controller.loadingServer}
            />
          </Flex>

          {!!server && (
            <ServerTileItem
              hideBanner
              server={server}
            />
          )}
        </Flex>
      </div>
    </TitleOutlet>
  );
});

function useDirectConnectPos(inputRef: React.RefObject<HTMLElement>): [number, number, number, number] {
  const [pos, setPos] = React.useState<[number, number, number, number]>([0, 0, 0, 0]);

  const calcPos = React.useCallback(() => {
    if (!inputRef.current) {
      return;
    }

    const rect = inputRef.current.getBoundingClientRect();

    setPos([rect.x, rect.y, rect.width, rect.height]);
  }, []);

  React.useLayoutEffect(calcPos, []);
  useWindowResize(calcPos);

  return pos;
}

function getDirectConnectLabel(config: ServerListConfigController, controller: ServerFiltersWithDirectConnectController): React.ReactNode {
  if (controller.server) {
    return (
      <>
        Connect to: <ServerTitle title={controller.server.projectName || controller.server.hostname} />
      </>
    );
  }

  if (controller.loadingServer) {
    return (
      <Flex>
        <Indicator />

        <span>
          Loading server data...
        </span>
      </Flex>
    );
  }

  if (config.searchText.length > 1) {
    if (!controller.parsedAddress) {
      return $L('#DirectConnect2_InvalidAddress');
    }

    return $L('#DirectConnect2_InvalidAddressOrOffline');
  }

  return $L('#DirectConnect2_EnterAddress');
}
