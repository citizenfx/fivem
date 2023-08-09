import { observer } from "mobx-react-lite";
import { mpMenu } from "../../mpMenu";
import { Pad } from "cfx/ui/Layout/Pad/Pad";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { TextBlock } from "cfx/ui/Text/Text";
import { FlexRestricter } from "cfx/ui/Layout/Flex/FlexRestricter";
import { Button } from "cfx/ui/Button/Button";
import { Icons } from "cfx/ui/Icons";
import { Title } from "cfx/ui/Title/Title";
import { Icon } from "cfx/ui/Icon/Icon";
import { CurrentGameBrand } from "cfx/base/gameRuntime";
import { useLegalService } from "cfx/apps/mpMenu/services/legal/legal.service";
import s from './LegalAccepter.module.scss';

export const LegalAccepter = observer(function TOSAccepter() {
  const legalService = useLegalService();

  return (
    <div className={s.root}>
      {/* The reason for the reverse order here is so that the "I Accept" button captures focus first when/if user press Tab key */}
      <Flex vertical fullHeight fullWidth reverseOrder gap="none">
        <Pad size="large">
          <Flex repell centered reverseOrder>
            <Button
              tabIndex={0}
              theme="primary"
              size="large"
              text="I Accept"
              onClick={legalService.accept}
            />

            <Title delay={1000} title={`Exit ${CurrentGameBrand}`}>
              <Button
                tabIndex={1}
                text="Cancel"
                onClick={mpMenu.exit}
              />
            </Title>
          </Flex>
        </Pad>

        <FlexRestricter vertical>
          <iframe
            src={`${legalService.TOS_URL}#toolbar=0&view=FitH`}
            className={s.iframe}
          ></iframe>
        </FlexRestricter>

        <Pad size="large">
          <Flex vertical gap="large">
            <TextBlock family="secondary" weight="bolder" size="xxlarge">
              Terms of Service
            </TextBlock>

            <Flex vertical gap="small">
              <TextBlock opacity="75">
                Last modified: {legalService.CURRENT_TOS_VERSION}
              </TextBlock>

              <TextBlock typographic opacity="75">
                <Title title={legalService.TOS_URL}>
                  <a href={legalService.TOS_URL}>Open the Terms of Service in your browser <Icon size="small">{Icons.externalLink}</Icon></a>
                </Title>
              </TextBlock>
            </Flex>
          </Flex>
        </Pad>
      </Flex>
    </div>
  );
});
