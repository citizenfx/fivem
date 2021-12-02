import * as React from 'react';
import classnames from 'classnames';
import { Input } from '../Input/Input';
import { Button } from '../Button/Button';
import { useOpenFolderSelectDialog } from 'utils/hooks';
import { BsBoxArrowUpRight } from 'react-icons/bs';
import s from './PathSelector.module.scss';

const defaultButtonIcon = <BsBoxArrowUpRight />;

export interface PathSelectorProps {
  value: string,
  onChange: (selectedPath: string) => void,
  startPath?: string,
  dialogTitle?: string,
  buttonText?: string,
  buttonIcon?: React.ReactNode,
  placeholder?: string,
  className?: string,
  disabled?: boolean,
  description?: React.ReactNode,
  notOnlyFolders?: boolean,
  showLoader?: boolean,
}

export const PathSelector = React.memo(function PathSelector(props: PathSelectorProps) {
  const {
    className,
    value,
    onChange,
    startPath = '',
    dialogTitle = 'Select Folder...',
    buttonText = 'Select',
    buttonIcon = defaultButtonIcon,
    placeholder = '',
    description = '',
    disabled = false,
    notOnlyFolders = false,
    showLoader = false,
  } = props;

  const folderSelectOptions = React.useMemo(() => ({
    startPath,
    dialogTitle,
    notOnlyFolders,
  }), [startPath, dialogTitle, notOnlyFolders]);

  const openFolderSelectDialog = useOpenFolderSelectDialog(folderSelectOptions, (folderPath) => {
    if (folderPath) {
      onChange(folderPath);
    }
  });

  const rootClassName = classnames(s.root, className);

  return (
    <div className={rootClassName}>
      <Input
        noSpellCheck
        className={s.input}
        value={value}
        onChange={onChange}
        placeholder={placeholder}
        disabled={disabled}
        description={description}
        showLoader={showLoader}
        decorator={() => (
          <Button
            text={buttonText}
            size="small"
            icon={buttonIcon}
            disabled={disabled}
            onClick={openFolderSelectDialog}
          />
        )}
      />
    </div>
  );
});
