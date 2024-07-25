/* eslint-disable react/no-unescaped-entities */
import {
  Icons,
  InfoPanel,
  Interactive,
  Flex,
  Pad,
  Scrollable,
  Modal,
  Symbols,
  Text,
  TextBlock,
  Title,
  ui,
  clsx,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { BsExclamationTriangleFill } from 'react-icons/bs';

import { useBoundingClientRect, useOpenFlag } from 'cfx/utils/hooks';

import { SearchInputController } from '../SearchInputController';

import s from './Cheatsheet.module.scss';

const SHOW_TIPS_TIMEOUT = 500;
const HIDE_TIPS_TIMEOUT = 1000;

export interface CheatsheetProps {
  controller: SearchInputController;
  inputRef: React.RefObject<HTMLDivElement>;
}

function useTipsShown(controller: SearchInputController): boolean {
  const [tipsShow, showTips, hideTips] = useOpenFlag(true);
  const timerRef = React.useRef<SetTimeoutReturn | null>(null);

  React.useEffect(() => {
    if (timerRef.current !== null) {
      clearTimeout(timerRef.current);
      timerRef.current = null;

      return;
    }

    const fn = controller.inputInFocus
      ? showTips
      : hideTips;
    const timeout = controller.inputInFocus
      ? SHOW_TIPS_TIMEOUT
      : HIDE_TIPS_TIMEOUT;

    timerRef.current = setTimeout(() => {
      timerRef.current = null;

      fn();
    }, timeout);

    return () => {
      if (timerRef.current !== null) {
        clearTimeout(timerRef.current);
      }
    };
  }, [controller.inputInFocus]);

  return tipsShow;
}

export const Cheatsheet = observer(function Cheatsheet(props: CheatsheetProps) {
  const {
    controller,
    inputRef,
  } = props;

  const [csOpen, openCs, closeCs] = useOpenFlag(false);

  const tipsShow = useTipsShown(controller);

  const inputRect = useBoundingClientRect(inputRef);

  if (controller.shouldRenderWizard || !inputRect) {
    return null;
  }

  const rootStyle: React.CSSProperties = {
    '--x': ui.px(inputRect.x),
    '--y': ui.px(inputRect.bottom),
    '--w': ui.px(inputRect.width),
  } as any;

  const rootClassName = clsx(s.root, {
    [s.active]: tipsShow,
  });

  return (
    <>
      <div style={rootStyle} className={rootClassName}>
        <Flex gap="small">
          <span>
            <kbd>"role play"</kbd> for full string match,
          </span>

          <span>
            <kbd>tag:</kbd> to match tags,
          </span>

          <span>
            <kbd>locale:</kbd> to match locale,
          </span>

          <span>
            <kbd>{'>'}127.13.37.0</kbd> for Direct Connect
          </span>
        </Flex>

        <Flex alignToEnd className={ui.cls.flexGrow}>
          <Interactive className={s.cheatsheetLink} onClick={openCs}>
            Cheat sheet
          </Interactive>
        </Flex>
      </div>

      {csOpen && (
        <CheatsheetModal onClose={closeCs} />
      )}
    </>
  );
});

interface CheatsheetModalProps {
  onClose(): void;
}
function CheatsheetModal(props: CheatsheetModalProps) {
  const {
    onClose,
  } = props;

  return (
    <Modal onClose={onClose}>
      <Scrollable>
        <Pad size="xlarge">
          <Flex vertical gap="xlarge">
            <Flex vertical gap="small">
              <Text size="large" opacity="75">
                Search terms:
              </Text>

              <TextBlock size="small" opacity="50">
                All search terms are divided by space: <kbd>en freeroam tag:race tag:trucks</kbd>
              </TextBlock>
            </Flex>

            <Flex>
              <div>
                <kbd>en</kbd>
              </div>

              <div>Quick locale filter, two letters of locale</div>
            </Flex>

            <Flex>
              <div>
                <kbd>term</kbd>
              </div>

              <Flex vertical gap="small">
                <span>Matches server's name and description</span>

                <TextBlock size="small" opacity="50">
                  For terms including space use quotes: <kbd>"role play"</kbd>
                </TextBlock>
              </Flex>
            </Flex>

            <Flex>
              <div>
                <kbd>
                  tag:<em>value</em>
                </kbd>
              </div>

              <Flex vertical gap="small">
                <span>Server tags filter</span>

                <TextBlock size="small" opacity="50">
                  Servers with zombie tag: <kbd>tag:zombie</kbd>
                </TextBlock>
              </Flex>
            </Flex>

            <Flex>
              <div>
                <kbd>
                  var:<em>value</em>
                </kbd>
              </div>

              <Flex vertical gap="small">
                <span>
                  Server <em>truthy</em> variables filter
                </span>

                <TextBlock size="small" opacity="50">
                  Servers with enabled scripthook: <kbd>var:sv_scriptHookEnabled</kbd>
                </TextBlock>
              </Flex>
            </Flex>

            <Flex>
              <div>
                <kbd>
                  locale:<em>value</em>
                </kbd>
              </div>

              <Flex vertical gap="small">
                <span>Full locale qualifier filter</span>

                <TextBlock size="small" opacity="50">
                  Servers with US english locale: <kbd>locale:en-US</kbd>
                </TextBlock>
              </Flex>
            </Flex>

            <Flex>
              <div>
                <kbd>
                  address:<em>value</em>
                </kbd>
              </div>

              <Flex vertical gap="small">
                <span>Server address filter</span>

                <TextBlock size="small" opacity="50">
                  Server address is the last part in join URL:{' '}
                  <kbd>
                    cfx.re/join/<strong>moonsc</strong>
                  </kbd>
                </TextBlock>
              </Flex>
            </Flex>

            <Flex>
              <div>
                <kbd>/regexp/</kbd>
              </div>

              <Flex vertical gap="small">
                <span>
                  <Title title={<>{Icons.externalLink} Open MDN RegExp documentation in browser</>}>
                    <a href="https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/RegExp">
                      RegExp
                    </a>
                  </Title>
                  {Symbols.nbsp}filter
                </span>

                <TextBlock size="small" opacity="50">
                  <em>Custom</em> and <em>races</em> inside of name or description: <kbd>/(custom|races?)/</kbd>
                </TextBlock>
              </Flex>
            </Flex>

            <InfoPanel type="success">
              <Flex vertical>
                <div>
                  RegExp can also be used as a value for both <kbd>tag:</kbd> and <kbd>var:</kbd>
                </div>

                <TextBlock size="small" opacity="75">
                  To match servers with either zombie or truck tag: <kbd>tag:/(zombie|truck)/</kbd>
                </TextBlock>
              </Flex>
            </InfoPanel>

            <Text size="large" opacity="75">
              Term qualifiers:
            </Text>

            <Flex>
              <div>
                <kbd>~</kbd>
              </div>

              <Flex vertical gap="small">
                <span>Exclude servers matching the following term</span>

                <TextBlock size="small" opacity="50">
                  To exclude servers with zombie tag: <kbd>~tag:zombie</kbd>
                </TextBlock>
              </Flex>
            </Flex>

            <Flex>
              <div>
                <kbd>{'>'}</kbd>
              </div>

              <Flex vertical gap="small">
                <span>Transforms input into Direct Connect server address input</span>

                <TextBlock size="small" opacity="50">
                  To connect to the local running server: <kbd>{'>'}127.0.0.1:30120</kbd>
                </TextBlock>
              </Flex>
            </Flex>

            <InfoPanel type="warning">
              <Flex vertical>
                <Flex>
                  <BsExclamationTriangleFill />

                  <div>Direct Connect qualifier can only be the very first character in the search input!</div>
                </Flex>

                <TextBlock size="small" opacity="75">
                  Search input string like this will <strong>not</strong> work as expected:{' '}
                  <kbd>freeroam {'>'}127.0.0.1:30120</kbd>
                </TextBlock>
              </Flex>
            </InfoPanel>
          </Flex>
        </Pad>
      </Scrollable>
    </Modal>
  );
}
