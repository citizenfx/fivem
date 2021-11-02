import React from 'react';
import { BsCardHeading } from 'react-icons/bs';
import { ShellViewParticipants } from 'fxdk/browser/shellExtensions';
import { ToolbarParticipants } from '../../toolbar/browser/toolbarExtensions';
import { UpdaterViewParticipants } from '../../updater/browser/updaterExtensions';
import { ChangelogModal } from './Changelog.modal';
import { hasNewChangelogEntries, setLatestChangelogEntryAsSeen } from './Changelog.utils';
import { ChangelogState } from './ChangelogState';
import { ChangelogUpdaterView } from './ChangelogUpdaterView';

ShellViewParticipants.register({
  id: 'changelog',
  isVisible: () => ChangelogState.isOpen,
  render: () => <ChangelogModal />,
});

UpdaterViewParticipants.register({
  id: 'changelog',
  shouldShowUpdater: hasNewChangelogEntries,
  handleUpdaterClose: setLatestChangelogEntryAsSeen,
  render: () => <ChangelogUpdaterView />,
});

ToolbarParticipants.registerMenuItem({
  id: 'changelog',
  group: 'misc',
  item: {
    id: 'changelog',
    text: 'Changelog',
    icon: <BsCardHeading />,
    onClick: ChangelogState.open,
  },
});
