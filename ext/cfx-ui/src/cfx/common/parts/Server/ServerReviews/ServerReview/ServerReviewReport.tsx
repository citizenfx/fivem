import React from "react";
import { $L } from "cfx/common/services/intl/l10n";
import { IServerReviewReport } from "cfx/common/services/servers/reviews/types";
import { Button } from "cfx/ui/Button/Button";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Pad } from "cfx/ui/Layout/Pad/Pad";
import { Modal } from "cfx/ui/Modal/Modal";
import { Radio } from "cfx/ui/Radio/Radio";
import { Textarea } from "cfx/ui/Textarea/Textarea";
import { Title } from "cfx/ui/Title/Title";
import { useOpenFlag } from "cfx/utils/hooks";
import { observer } from "mobx-react-lite";
import { BsFlag } from "react-icons/bs";

export interface ServerReviewReportProps {
  report: IServerReviewReport,
  className?: string,
}

export const ServerReviewReport = observer(function ServerReviewReport(props: ServerReviewReportProps) {
  const {
    report,
    className,
  } = props;

  const [isModalOpen, openModal, closeModal] = useOpenFlag(false);
  const [optionValue, setOptionValue] = React.useState(report.options[0].value);
  const [message, setMessage] = React.useState('');

  const currentOption = report.options.find(({ value }) => value === optionValue)!;

  const handleSendReport = () => {
    if (!currentOption) {
      return;
    }

    report.submit(
      currentOption,
      currentOption.withMessage
        ? message
        : undefined,
    );

    closeModal();
  };

  return (
    <>
      <Title title={$L('#Review_Flag')}>
        <Button
          theme="transparent"
          size="small"
          icon={<BsFlag />}
          className={className}
          onClick={openModal}
        />
      </Title>

      {isModalOpen && (
        <Modal onClose={closeModal}>
          <Modal.Header>
            {$L('#Review_Flag')}
          </Modal.Header>

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
        </Modal>
      )}
    </>
  );
});
