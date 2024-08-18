import {
  Button,
  Indicator,
  InfoPanel,
  Flex,
  Pad,
  Modal,
  Radio,
  Text,
  Textarea,
  Title,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { BsFlag } from 'react-icons/bs';

import { $L } from 'cfx/common/services/intl/l10n';
import { IServerReviewReport } from 'cfx/common/services/servers/reviews/types';
import { useOpenFlag } from 'cfx/utils/hooks';

export interface ServerReviewReportProps {
  report: IServerReviewReport;
  className?: string;
}

export const ServerReviewReport = observer(function ServerReviewReport(props: ServerReviewReportProps) {
  const {
    report,
    className,
  } = props;

  const [isModalOpen, openModal, closeModal] = useOpenFlag(false);

  return (
    <>
      <Title title={$L('#Review_Flag')}>
        <Button theme="transparent" size="small" icon={<BsFlag />} className={className} onClick={openModal} />
      </Title>

      {isModalOpen && (
        <Modal onClose={closeModal}>
          <Modal.Header>{$L('#Review_Flag')}</Modal.Header>

          <ServerReviewReportModal onClose={closeModal} report={report} />
        </Modal>
      )}
    </>
  );
});

interface ServerReviewReportModalProps {
  onClose(): void;
  report: IServerReviewReport;
}
const ServerReviewReportModal = observer(function ServerReviewReportModal(props: ServerReviewReportModalProps) {
  const {
    onClose,
    report,
  } = props;

  React.useEffect(() => {
    // Ensure options load requested
    report.ensureOptions();
  }, [report]);

  if (report.optionsLoading) {
    return (
      <>
        <Pad>
          <Flex>
            <Indicator />
            <Text>{$L('#Review_Flag_OptionsLoading')}</Text>
          </Flex>
        </Pad>

        <Modal.Footer>
          <Button text={$L('#Review_Flag_Cancel')} onClick={onClose} />
        </Modal.Footer>
      </>
    );
  }

  if (report.optionsError || !report.options.length) {
    return (
      <>
        <Pad>
          <InfoPanel type="error">{$L('#Review_Flag_Unavailable')}</InfoPanel>
        </Pad>

        <Modal.Footer>
          <Button text={$L('#Review_Flag_Cancel')} onClick={onClose} />
        </Modal.Footer>
      </>
    );
  }

  return (
    <ServerReviewReportOptions onClose={onClose} report={report} />
  );
});

interface ServerReviewReportOptionsProps {
  onClose(): void;
  report: IServerReviewReport;
}
const ServerReviewReportOptions = observer(function ServerReviewReportOptions(props: ServerReviewReportOptionsProps) {
  const {
    onClose,
    report,
  } = props;

  const [optionValue, setOptionValue] = React.useState(report.options[0].value);
  const [message, setMessage] = React.useState('');

  const currentOption = React.useMemo(() => report.options
    .find(({
      value,
    }) => value === optionValue)!, [report.options, optionValue]);

  const handleSendReport = () => {
    if (!currentOption) {
      return;
    }

    report.submit(currentOption, currentOption.withMessage
      ? message
      : undefined);

    onClose();
  };

  return (
    <>
      <Pad>
        <Flex vertical>
          {report.options.map((option) => (
            <Radio
              key={option.id}
              label={option.label}
              checked={optionValue === option.value}
              onChange={() => setOptionValue(option.value)}
            />
          ))}

          {currentOption.withMessage && (
            <Textarea
              autofocus
              rows={5}
              value={message}
              onChange={setMessage}
              placeholder={currentOption.messagePlaceholder}
            />
          )}
        </Flex>
      </Pad>

      <Modal.Footer>
        <Button
          text={$L('#Review_FlagCTA')}
          theme="primary"
          disabled={!report.canSubmit(currentOption, message)}
          onClick={handleSendReport}
        />
      </Modal.Footer>
    </>
  );
});
