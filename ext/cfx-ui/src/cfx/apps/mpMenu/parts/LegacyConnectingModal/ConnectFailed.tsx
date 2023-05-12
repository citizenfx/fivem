import React from "react";
import { mpMenu } from "cfx/apps/mpMenu/mpMenu";
import { ConnectState } from "cfx/apps/mpMenu/services/servers/connect/state";
import { CurrentGameBrand, CurrentGameName } from "cfx/base/gameRuntime";
import { createPlaceholderIconDataURI } from "cfx/base/placeholderIcon";
import { ServerIcon } from "cfx/common/parts/Server/ServerIcon/ServerIcon";
import { useAccountService } from "cfx/common/services/account/account.service";
import { $L, useL10n } from "cfx/common/services/intl/l10n";
import { IServerView } from "cfx/common/services/servers/types";
import { Avatar } from "cfx/ui/Avatar/Avatar";
import { Button } from "cfx/ui/Button/Button";
import { ButtonBar } from "cfx/ui/Button/ButtonBar";
import { ControlBox } from "cfx/ui/ControlBox/ControlBox";
import { Icon } from "cfx/ui/Icon/Icon";
import { BrandIcon } from "cfx/ui/Icons";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Pad } from "cfx/ui/Layout/Pad/Pad";
import { Separator } from "cfx/ui/Separator/Separator";
import { Text, TextBlock, TextColor } from "cfx/ui/Text/Text";
import { noop } from "cfx/utils/functional";
import { html2react } from "cfx/utils/html2react";
import { linkify } from "cfx/utils/links";
import { nl2br } from "cfx/utils/nl2br";
import { observer } from "mobx-react-lite";
import { BsCheckCircle, BsFillQuestionCircleFill, BsXCircleFill } from "react-icons/bs";
import { MdWrongLocation } from "react-icons/md";
import { replaceCfxRePlaceholders, useRenderedFormattedMessage } from "../../utils/messageFormatting";
import { usePlatformStatusService } from "../../services/platformStatus/platformStatus.service";
import { StatusLevel } from "../../services/platformStatus/types";
import { Modal } from "cfx/ui/Modal/Modal";
import { useStreamerMode } from "../../services/convars/convars.service";

type ConnectFailedProps = {
  state: ConnectState.Failed,
  server: IServerView,
  onClose?(): void,
}
export const ConnectFailed = observer(function ConnectFailed(props: ConnectFailedProps) {
  const {
    state,
    server,
    onClose = noop,
  } = props;

  const {
    extra,
    extraActions,
    title = '#Servers_ConnectFailed',
  } = state;

  const stateMessage = useRenderedFormattedMessage(state, server);
  const serviceStatus = useServiceStatus(extra?.status);
  const bodyLocalized = serviceStatus
    ? useL10n(serviceStatus?.body)
    : '';

  return (
    <>
      <Pad size="large">
        <Flex vertical gap="large" fullWidth>
          {!!extra?.fault && (
            <FailureScheme
              fault={extra.fault}
              server={server}
            />
          )}

          <Text size="xlarge" color="error">
            {$L(title)}
          </Text>

          {!!serviceStatus && (
            <Flex vertical>
              <Text userSelectable size="large">
                {$L(serviceStatus.title)}
              </Text>

              <TextBlock typographic userSelectable>
                {html2react(linkify(nl2br(bodyLocalized)))}
              </TextBlock>
            </Flex>
          )}

          {!!extra?.action && (
            <Flex vertical>
              <Text size="large">
                {$L('#ErrorActions')}
              </Text>

              <StateExtraAction
                server={server}
                action={extra.action}
              />
            </Flex>
          )}

          <Flex vertical fullWidth>
            <Text size="large">
              {$L('#ErrorDetails')}
            </Text>

            <TextBlock typographic userSelectable>
              {stateMessage}
            </TextBlock>
          </Flex>
        </Flex>
      </Pad>

      <Modal.Footer>
        <Flex repell>
          {!!extraActions?.length && (
            <ButtonBar>
              {extraActions.map((extraAction) => (
                <Button
                  key={extraAction.id}
                  text={extraAction.label}
                  onClick={extraAction.action}
                />
              ))}
            </ButtonBar>
          )}

          <Button
            theme={extraActions?.length ? 'transparent' : 'default'}
            text={$L('#Servers_CloseOverlay')}
            onClick={onClose}
          />
        </Flex>
      </Modal.Footer>
    </>
  );
});

function useServiceStatus(showStatusAnyway: boolean | undefined | null): { title: string, body: string } | null {
  const PlatformStatusService = usePlatformStatusService();

  const level = PlatformStatusService.level;

  // Refresh platform status
  React.useEffect(() => {
    PlatformStatusService.fetchStatus();
  }, []);

  if (showStatusAnyway && level < StatusLevel.MinorOutage) {
    return {
      title: '#ErrorNoOutage',
      body: '#NoOutage',
    };
  }

  if (level >= StatusLevel.MajorOutage) {
    return {
      title: '#ErrorOutage',
      body: '#ServiceAlert',
    };
  }

  return null;
}

const StateExtraAction = observer(function StateExtraAction(props: { action: string, server: IServerView }) {
  const { action, server } = props;

  const actionLocalized = useL10n(action);

  const actionx = React.useMemo(() => {
    return html2react(
      linkify(
        nl2br(
          replaceCfxRePlaceholders(
            actionLocalized,
            server,
          ),
        ),
      ),
    );
  }, [actionLocalized, server]);

  return (
    <TextBlock typographic>
      {actionx}
    </TextBlock>
  );
});

function isCfxFault(fault: string): boolean {
  return fault.startsWith('cfx');
}

const FailureScheme = observer(function FailureScheme(props: { server: IServerView, fault: NonNullable<ConnectState.Failed['extra']>['fault'] }) {
  const {
    fault,
    server,
  } = props;

  const AccountService = useAccountService();

  const streamerMode = useStreamerMode();

  const userAvatarURL = AccountService.account
    ? AccountService.account.getAvatarUrl()
    : createPlaceholderIconDataURI(mpMenu.getPlayerNickname());

  const userToCfx = fault === 'you'
    ? 'bye'
    : (
      fault === 'either'
        ? 'unknown'
        : (
          isCfxFault(fault)
            ? 'broken'
            : 'ok'
        )
    );

  const cfxToServer = fault === 'you'
    ? 'bye'
    : (
      (fault === 'either' || isCfxFault(fault))
        ? 'unknown'
        : 'broken'
    );

  return (
    <Flex repell fullWidth centered gap="large">
      <Flex vertical centered>
        <Avatar
          size="large"
          url={streamerMode ? null : userAvatarURL}
        />

        <Text size="large">
          {$L('#Error_You')}
        </Text>
      </Flex>

      <Separator
        content={<ConnectivityIcon state={userToCfx} />}
      />

      <Flex vertical centered>
        <Text size="xxlarge">
          {BrandIcon[CurrentGameName]}
        </Text>

        <Text size="large">
          {CurrentGameBrand}
        </Text>
      </Flex>

      <Separator
        content={<ConnectivityIcon state={cfxToServer} />}
      />

      <Flex vertical centered>
        <ControlBox size="large">
          <ServerIcon
            type="list"
            server={server}
          />
        </ControlBox>

        <Text size="large">
          {$L('#Error_Server')}
        </Text>
      </Flex>
    </Flex>
  );
});

const iconsMap: Record<string, { icon: React.ReactNode, color: TextColor }> = {
  ok: {
    icon: <BsCheckCircle />,
    color: 'success',
  },
  unknown: {
    icon: <BsFillQuestionCircleFill />,
    color: 'teal',
  },
  broken: {
    icon: <BsXCircleFill />,
    color: 'error',
  },
  bye: {
    icon: <MdWrongLocation />,
    color: 'primary',
  },
};
function ConnectivityIcon(props: { state: 'ok' | 'unknown' | 'broken' | 'bye' }) {
  const { icon, color } = iconsMap[props.state];

  return (
    <Icon color={color} size="xxlarge">
      {icon}
    </Icon>
  );
}
