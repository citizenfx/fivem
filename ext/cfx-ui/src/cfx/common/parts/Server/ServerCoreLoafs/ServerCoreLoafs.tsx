import React from "react";
import copy from "copy-to-clipboard";
import { getGameBuildDLCName } from "cfx/base/game";
import { $L } from "cfx/common/services/intl/l10n";
import { IServerView, ServerPureLevel } from "cfx/common/services/servers/types";
import { Avatar } from "cfx/ui/Avatar/Avatar";
import { Badge } from "cfx/ui/Badge/Badge";
import { LinkButton } from "cfx/ui/Button/LinkButton";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Symbols } from "cfx/ui/Symbols";
import { Text } from "cfx/ui/Text/Text";
import { Title } from "cfx/ui/Title/Title";
import { getValue, ValueOrGetter } from "cfx/utils/getValue";
import { useDynamicRef, useOpenFlag } from "cfx/utils/hooks";
import { observer } from "mobx-react-lite";
import { AiFillCrown } from "react-icons/ai";
import { BsLockFill, BsMap } from "react-icons/bs";
import { IoGameController } from "react-icons/io5";
import { ServerBoostButton } from "../ServerBoostButton/ServerBoostButton";
import { Loaf } from "cfx/ui/Loaf/Loaf";
import { Button } from "cfx/ui/Button/Button";
import { BiCopy } from "react-icons/bi";
import s from './ServerCoreLoafs.module.scss';

interface IExtraLoafDescriptor {
  key: keyof IServerView,
  title?: React.ReactNode | ((value: string) => React.ReactNode),
  icon?: ValueOrGetter<React.ReactNode>,
  value?(value: string): React.ReactNode,
}

const EXTRA_DETAIL_BITS: IExtraLoafDescriptor[] = [
  {
    key: 'mapname',
    title: 'Map',
    icon: <BsMap />,
  },
  {
    key: 'gametype',
    title: 'Game type',
    icon: <IoGameController />,
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
    title: (level: ServerPureLevel) => ({
      [ServerPureLevel.None]: 'Client mods allowed',
      [ServerPureLevel.AudioAndGraphicsAllowed]: 'Only audio and graphic client mods allowed',
      [ServerPureLevel.NoModsAllowed]: 'Client mods disallowed',
    }[level]),
    value: (level: ServerPureLevel) => {
      if (!level || level === ServerPureLevel.None) {
        return;
      }

      return (
        <>
          Pure Mode <Badge>L{level}</Badge>
        </>
      );
    },
  }
];

function getTitle(title: IExtraLoafDescriptor['title'], value: string): React.ReactNode {
  if (typeof title === 'function') {
    return title(value);
  }

  return title;
}

export interface ServerCoreLoafsProps {
  server: IServerView,

  hideActions?: boolean,
}

export const ServerCoreLoafs = observer(function ServerCoreLoafs(props: ServerCoreLoafsProps) {
  const {
    server,
    hideActions = false,
  } = props;

  const nodes: React.ReactNode[] = [];

  if (!hideActions && server.ownerName && server.ownerProfile) {
    const { ownerName, ownerProfile, ownerAvatar } = server;

    const titleNode = (
      <Flex centered>
        {!!ownerAvatar && (
          <Avatar url={ownerAvatar} />
        )}
        <Flex vertical gap="small">
          <Text color="inherit" weight="bold">
            {ownerName}
          </Text>

          Open server owner's profile in browser
        </Flex>
      </Flex>
    );

    nodes.push(
      <Title key="owner-link" title={titleNode}>
        <LinkButton
          to={ownerProfile}
          size="small"
          icon={<AiFillCrown />}
          text={ownerName}
        />
      </Title>
    );
  }

  if (!hideActions && server.joinId && !server.private) {
    nodes.push(
      <Copier
        key="copy-join-id"
        text={`cfx.re/join/${server.joinId}`}
      />
    );

    nodes.push(
      <ServerBoostButton
        key="boost-button"
        server={server}
      />
    );
  }

  if (server.onesyncEnabled) {
    nodes.push(
      <Title key="onesync" title="OneSync enabled">
        <Loaf className={s.onesync} bright size="small">
          <span className={s.plz}>
            <span>O</span>
            <span>n</span>
            <span>e</span>
            <span>S</span>
            ync
          </span>
        </Loaf>
      </Title>
    );
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
      </Title>
    );
  }

  return <>{nodes}</>;
});

function Copier({ text }) {
  const [copied, setCopied, unsetCopied] = useOpenFlag(false);

  const textRef = useDynamicRef(text);
  const timerRef = React.useRef<SetTimeoutReturn | null>(null);
  const unsetCopiedRef = useDynamicRef(unsetCopied);

  const handleClick = React.useCallback(() => {
    copy(textRef.current);

    if (timerRef.current !== null) {
      clearTimeout(timerRef.current);
    }

    timerRef.current = setTimeout(() => {
      timerRef.current = null;
      unsetCopiedRef.current();
    }, 1000);

    setCopied();
  }, []);

  React.useEffect(() => () => {
    if (timerRef.current !== null) {
      clearTimeout(timerRef.current);
    }
  }, []);

  const title = copied
    ? 'Copied!'
    : 'Copy to clipboard';

  return (
    <Title key="join-link-copier" title={title}>
      <Button
        size="small"
        icon={<BiCopy />}
        text={text}
        onClick={handleClick}
      />
    </Title>
  );
}
