import React from "react";
import { observer } from "mobx-react-lite";
import { getGameBuildDLCName } from "cfx/base/game";
import { $L } from "cfx/common/services/intl/l10n";
import { IServerView, ServerPureLevel } from "cfx/common/services/servers/types";
import { Avatar } from "cfx/ui/Avatar/Avatar";
import { LinkButton } from "cfx/ui/Button/LinkButton";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Loaf } from "cfx/ui/Loaf/Loaf";
import { Text } from "cfx/ui/Text/Text";
import { Title } from "cfx/ui/Title/Title";
import { getValue, ValueOrGetter } from "cfx/utils/getValue";
import { defaultLinkReplacerx, linkifyx } from "cfx/utils/links";
import { AiFillCrown } from "react-icons/ai";
import { BsLockFill, BsMap } from "react-icons/bs";
import { FiExternalLink } from "react-icons/fi";
import { IoGameController } from "react-icons/io5";
import { Badge } from "cfx/ui/Badge/Badge";
import { Symbols } from "cfx/ui/Symbols";
import { Button } from "cfx/ui/Button/Button";
import { BiCopy } from 'react-icons/bi';
import s from './ServerExtraDetails.module.scss';
import copy from "copy-to-clipboard";

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

export const ServerExtraDetails = observer(function ServerExtraDetails({ server }: { server: IServerView }) {
  const nodes: React.ReactNode[] = [];

  if (server.ownerName && server.ownerProfile) {
    const { ownerName, ownerProfile, ownerAvatar } = server;

    const titleNode = (
      <Flex centered>
        {!!ownerAvatar && <Avatar url={ownerAvatar} />}
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

    const joinLink = `cfx.re/join/${server.address}`;

    nodes.push(
      <Title key="join-link-copier" title="Copy to clipboard">
        <Button
          size="small"
          icon={<BiCopy />}
          text={joinLink}
          onClick={() => copy(joinLink)}
        />
      </Title>
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

  const varNodes: React.ReactNode[] = [];

  if (server.variables) {
    for (const [name, value] of Object.entries(server.variables)) {
      varNodes.push(
        <Flex key={name + value}>
          <Text opacity="75">{name}:</Text>
          <span>
            <Linkify text={value} />
          </span>
        </Flex>
      );
    }
  }

  return (
    <Flex vertical gap="large">
      <Flex wrap>
        {nodes}
      </Flex>

      {Boolean(varNodes.length) && (
        <Flex vertical>
          {varNodes}
        </Flex>
      )}
    </Flex>
  );
});

const linkReplacer: typeof defaultLinkReplacerx = (key, text, url) => (
  <Title key={key} title={<><FiExternalLink /> {url}</>}>
    <a href={url}>
      {text.replace('http://', '').replace('https://', '')}
    </a>
  </Title>
);

function Linkify({ text }: { text: string }) {
  const linkified = React.useMemo(() => linkifyx(text, linkReplacer), [text]);

  return linkified as any;
}
