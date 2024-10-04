import {
  Avatar,
  Button,
  Indicator,
  Input,
  Box,
  Flex,
  Pad,
  Radio,
  Text,
  TextBlock,
  Textarea,
  Title,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';

import { useAccountService } from 'cfx/common/services/account/account.service';
import { $L } from 'cfx/common/services/intl/l10n';
import { IServerReviews } from 'cfx/common/services/servers/reviews/types';
import { IServerView } from 'cfx/common/services/servers/types';
import { proxyInvariant } from 'cfx/utils/invariant';

import { useServerReviewFormState } from './ServerReviewFormState';
import { ServerTitle } from '../../ServerTitle/ServerTitle';

import s from './ServerReviewForm.module.scss';

export const ServerReviewFormContext = React.createContext({
  censorUser: false,
});

export interface ServerReviewFormProps {
  server: IServerView;
  serverReviews: IServerReviews;
}
export const ServerReviewForm = observer(function ServerReviewForm(props: ServerReviewFormProps) {
  const {
    server,
    serverReviews,
  } = props;

  const account = proxyInvariant(useAccountService().account, 'No account!');
  const {
    censorUser,
  } = React.useContext(ServerReviewFormContext);

  const state = useServerReviewFormState();
  // ---
  state.serverReviews = serverReviews;
  // ---

  const username = censorUser
    ? '<HIDDEN>'
    : account.username;
  const playtime = serverReviews.ownPlaytime?.formattedSeconds || $L('#Reviews_NeverPlayed');

  return (
    <Box className={s.root}>
      <Pad>
        <Flex vertical gap="large">
          <Text size="xlarge">
            {$L('#Review_WriteAbout_Prefix')}
            &nbsp;
            <ServerTitle size="xlarge" title={server.projectName || server.hostname} />
          </Text>

          <TextBlock typographic opacity="50">
            Please describe what you liked or disliked about this server and whether you recommend it to others.
            <br />
            To report misconduct regarding this server, please{' '}
            <a href="https://support.cfx.re/">open an abuse ticket</a> instead.
          </TextBlock>

          <Flex>
            <Box width={30} noShrink noOverflow>
              <Flex vertical repell fullHeight>
                <Flex>
                  <Avatar url={censorUser
                    ? null
                    : account.getAvatarUrl()}
                  />

                  <Box grow>
                    <Flex vertical gap="small">
                      <Title fixedOn="bottom-left" title={username}>
                        <Text truncated typographic>
                          {username}
                        </Text>
                      </Title>

                      <Title title="Time user spent on server">
                        <TextBlock size="small" opacity="75">
                          {playtime}
                        </TextBlock>
                      </Title>
                    </Flex>
                  </Box>
                </Flex>
              </Flex>
            </Box>

            <Box grow>
              <Flex vertical gap="large">
                <Flex vertical gap="small">
                  <Box width="50%">
                    {' '}
                    {/* Force input to be of auto width instead of growing full width of the container */}
                    <Input
                      fullWidth
                      size="large"
                      value={state.title}
                      disabled={state.disabled}
                      onChange={state.setTitle}
                      placeholder="Title"
                    />
                  </Box>

                  <Text size="xsmall" opacity="50">
                    Minimum 5 characters
                  </Text>
                </Flex>

                <Flex vertical gap="small">
                  <Textarea
                    rows={5}
                    value={state.content}
                    disabled={state.disabled}
                    onChange={state.setContent}
                    placeholder="Write your review here—remember to be polite and constructive."
                  />

                  <Text size="xsmall" opacity="50">
                    Minimum 10 characters
                  </Text>
                </Flex>

                <Flex vertical>
                  <Text opacity={state.submitting
                    ? '50'
                    : '100'}
                  >Do you recommend this server?
                  </Text>

                  <Flex repell centered="axis">
                    <Flex>
                      <Radio
                        checked={state.recommend === true}
                        label={$L('#Yes')}
                        onChange={state.setRecommend}
                        disabled={state.disabled}
                      />
                      <Radio
                        checked={state.recommend === false}
                        label={$L('#No')}
                        onChange={state.setNotRecommend}
                        disabled={state.disabled}
                      />
                    </Flex>

                    {state.submitting && (
                      <Indicator />
                    )}

                    <Button text="Submit review" theme="primary" disabled={!state.canSubmit} onClick={state.submit} />
                  </Flex>
                </Flex>
              </Flex>
            </Box>
          </Flex>
        </Flex>
      </Pad>
    </Box>
  );
});
