import React from "react";
import { observer } from "mobx-react-lite";
import { IServerView } from "cfx/common/services/servers/types";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Text } from "cfx/ui/Text/Text";
import { Title } from "cfx/ui/Title/Title";
import { defaultLinkReplacerx, linkifyx } from "cfx/utils/links";
import { FiExternalLink } from "react-icons/fi";
import { ServerCoreLoafs } from "../ServerCoreLoafs/ServerCoreLoafs";
import { ui } from "cfx/ui/ui";
import { Icons } from "cfx/ui/Icons";

export const ServerExtraDetails = observer(function ServerExtraDetails({ server }: { server: IServerView }) {
  const varNodes: React.ReactNode[] = [];

  if (server.variables) {
    for (const [name, value] of Object.entries(server.variables)) {
      varNodes.push(
        <Flex key={name + value}>
          <Text opacity="75" className={ui.cls.flexNoShrink}>{name}:</Text>
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
        <ServerCoreLoafs server={server} />
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
  <Title key={key} title={<>{Icons.externalLink} {url}</>}>
    <a href={url}>
      {text.replace('http://', '').replace('https://', '')}
    </a>
  </Title>
);

function Linkify({ text }: { text: string }) {
  const linkified = React.useMemo(() => linkifyx(text, linkReplacer), [text]);

  return linkified as any;
}
