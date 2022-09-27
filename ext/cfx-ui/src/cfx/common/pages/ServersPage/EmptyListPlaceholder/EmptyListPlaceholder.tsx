import { ServersListType } from "cfx/common/services/servers/lists/types";
import { LinkButton } from "cfx/ui/Button/LinkButton";
import { Island } from "cfx/ui/Island/Island";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Page } from "cfx/ui/Layout/Page/Page";
import { Text } from "cfx/ui/Text/Text";
import { observer } from "mobx-react-lite";
import { WiWindy } from "react-icons/wi";
import { SERVER_LIST_DESCRIPTORS } from "../ListTypeTabs";
import s from './EmptyListPlaceholder.module.scss';

export const EmptyListPlaceholder = observer(function EmptyListPlaceholder() {
  return (
    <Page>
      <Island grow>
        <Flex fullWidth fullHeight vertical centered gap="xlarge">
          <WiWindy className={s.icon} />

          <Text size="xlarge" weight="bold" opacity="75">
            Looks pretty empty here
          </Text>

          <LinkButton
            to={SERVER_LIST_DESCRIPTORS[ServersListType.All].to}
            text={SERVER_LIST_DESCRIPTORS[ServersListType.All].title}
            size="large"
            theme="primary"
          />
        </Flex>
      </Island>
    </Page>
  );
});
