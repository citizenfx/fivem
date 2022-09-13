import React from "react";
import { ServerFilters } from "cfx/common/parts/Server/ServerFilters/ServerFilters";
import { ServerListConfigController } from "cfx/common/services/servers/lists/ServerListConfigController";
import { observer } from "mobx-react-lite";
import { useWindowResize } from "cfx/utils/hooks";
import { ui } from "cfx/ui/ui";
import { ServerListItem } from "cfx/common/parts/Server/ServerListItem/ServerListItem";
import { ServerFiltersWithDirectConnectController, userServerFiltersWithDirectConnectController } from "./ServerFiltersWithDirectConnectController";
import { Indicator } from "cfx/ui/Indicator/Indicator";
import { TitleOutlet } from "cfx/ui/outlets";
import { Shroud } from "cfx/ui/Shroud/Shroud";
import { Box } from "cfx/ui/Layout/Box/Box";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Text } from "cfx/ui/Text/Text";
import { Button } from "cfx/ui/Button/Button";
import { Pad } from "cfx/ui/Layout/Pad/Pad";
import { ServerTitle } from "cfx/common/parts/Server/ServerTitle/ServerTitle";
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

  const label = server
    ? <>Connect to: <ServerTitle title={server.projectName || server.hostname} /></>
    : (
      controller.loadingServer
        ? 'Loading server data...'
        : 'Enter server address'
    );

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
            <Text opacity="75">
              {label}
            </Text>

            <Button
              size="small"
              theme="primary"
              text="Press Enter to connect"
              disabled={!server || controller.loadingServer}
            />
          </Flex>

          <Box
            height={10}
            width="100%"
            style={{ backgroundColor: 'var(--color-input-background)', borderRadius: 'var(--border-radius-small)' }}
          >
            {!!server && (
              <ServerListItem
                standalone
                hideTags
                hideActions
                hideCountryFlag
                hidePremiumBadge
                server={server}
              />
            )}

            {!server && (
              <Flex centered="axis" fullHeight>
                <Pad left size="large" >
                  {
                    controller.loadingServer
                      ? <Indicator />
                      : (
                        !!controller.parsedAddress
                          ? <>No such server</>
                          : <>Invalid server address</>
                      )
                  }
                </Pad>
              </Flex>
            )}
          </Box>
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
