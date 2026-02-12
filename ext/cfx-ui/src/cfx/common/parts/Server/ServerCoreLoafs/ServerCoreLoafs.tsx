import {
  Avatar,
  Badge,
  Button,
  Flex,
  Icons,
  Loaf,
  Symbols,
  Text,
  Title,
  ValueOrGetter,
  getValue,
  useDynamicRef,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { BsLockFill } from 'react-icons/bs';

import { getGameBuildDLCName } from 'cfx/base/game';
import { AnalyticsLinkButton } from 'cfx/common/parts/AnalyticsLinkButton/AnalyticsLinkButton';
import { ElementPlacements } from 'cfx/common/services/analytics/types';
import { $L } from 'cfx/common/services/intl/l10n';
import { IServerView, ServerPureLevel } from 'cfx/common/services/servers/types';
import { useOpenFlag } from 'cfx/utils/hooks';

import { ServerBoostButton } from '../ServerBoostButton/ServerBoostButton';

interface IExtraLoafDescriptor {
  key: keyof IServerView;
  title?: React.ReactNode | ((value: string) => React.ReactNode);
  icon?: ValueOrGetter<React.ReactNode>;
  value?(value: string): React.ReactNode;
}

const EXTRA_DETAIL_BITS: IExtraLoafDescriptor[] = [
  {
    key: 'mapname',
    title: $L('#ServerDetail_Info_Mapname'),
    icon: Icons.map,
  },
  {
    key: 'gametype',
    title: $L('#ServerDetail_Info_Gametype'),
    icon: Icons.controller,
  },
  {
    key: 'enforceGameBuild',
    title: (value) => (
      <>
        {$L('#ServerDetail_DLCLevel')} {Symbols.htmlDot} b{value}
      </>
    ),
    value: (value) => {
      const humanReadableDLCName = getGameBuildDLCName(value);

      if (humanReadableDLCName) {
        return `DLC: ${humanReadableDLCName}`;
      }

      return '';
    },
  },
  {
    key: 'pureLevel',
    icon: <BsLockFill />,
    title: (level: ServerPureLevel) => $L(
      `#ServerDetail_Info_PureLevel_${level}`,
      undefined,
      `Pure Mode Level ${level}`,
    ),
    value: (level: ServerPureLevel) => {
      if (!level || level === ServerPureLevel.None) {
        return null;
      }

      return (
        <>
          {$L('#ServerDetail_Info_PureLevel')} <Badge>L{level}</Badge>
        </>
      );
    },
  },
];

function getTitle(title: IExtraLoafDescriptor['title'], value: string): React.ReactNode {
  if (typeof title === 'function') {
    return title(value);
  }

  return title;
}

export interface ServerCoreLoafsProps {
  server: IServerView;
  hideActions?: boolean;
  elementPlacement?: ElementPlacements;
}

export const ServerCoreLoafs = observer(function ServerCoreLoafs(props: ServerCoreLoafsProps) {
  const {
    server,
    elementPlacement,
    hideActions = false,
  } = props;

  const nodes: React.ReactNode[] = [];

  if (!hideActions && server.ownerName && server.ownerProfile) {
    const {
      ownerName,
      ownerProfile,
      ownerAvatar,
    } = server;

    const titleNode = (
      <Flex centered>
        {!!ownerAvatar && (
          <Avatar url={ownerAvatar} />
        )}
        <Flex vertical gap="small">
          <Text color="inherit" weight="bold">
            {ownerName}
          </Text>

          {$L('#ServerDetail_Info_OpenOwnerProfile_Title')}
        </Flex>
      </Flex>
    );

    nodes.push(
      <Title key="owner-link" title={titleNode}>
        <AnalyticsLinkButton
          to={ownerProfile}
          size="small"
          icon={Icons.crown}
          text={ownerName}
          elementPlacement={elementPlacement}
        />
      </Title>,
    );
  }

  if (!hideActions && server.joinId && !server.private) {
    nodes.push(<Copier key="copy-join-id" text={`cfx.re/join/${server.joinId}`} />);

    nodes.push(<ServerBoostButton key="boost-button" server={server} />);
  }

  for (const extraLoaf of EXTRA_DETAIL_BITS) {
    const value = server[extraLoaf.key];

    const displayValue = extraLoaf.value
      ? extraLoaf.value(value)
      : value;

    if (!displayValue) {
      continue;
    }

    nodes.push(
      <Title key={extraLoaf.key} title={getTitle(extraLoaf.title, value)}>
        <Loaf bright size="small">
          {!!extraLoaf.icon && getValue(extraLoaf.icon)}
          {displayValue}
        </Loaf>
      </Title>,
    );
  }

  return (
    // eslint-disable-next-line react/jsx-no-useless-fragment
    <>{nodes}</>
  );
});

function Copier({
  text,
}: { text: string }) {
  const [copied, setCopied, unsetCopied] = useOpenFlag(false);

  const textRef = useDynamicRef(text);
  const timerRef = React.useRef<SetTimeoutReturn | null>(null);
  const unsetCopiedRef = useDynamicRef(unsetCopied);

  const handleClick = React.useCallback(() => {
    navigator.clipboard.writeText(textRef.current);

    if (timerRef.current !== null) {
      clearTimeout(timerRef.current);
    }

    timerRef.current = setTimeout(() => {
      timerRef.current = null;
      unsetCopiedRef.current();
    }, 1000);

    setCopied();
  }, []);

  React.useEffect(
    () => () => {
      if (timerRef.current !== null) {
        clearTimeout(timerRef.current);
      }
    },
    [],
  );

  if ('clipboard' in navigator === false) {
    return null;
  }

  const title = copied
    ? $L('#ServerDetail_Info_CopyJoinId_Done')
    : $L('#ServerDetail_Info_CopyJoinId');

  return (
    <Title key="join-link-copier" title={title}>
      <Button size="small" icon={Icons.copy} text={text} onClick={handleClick} />
    </Title>
  );
}
