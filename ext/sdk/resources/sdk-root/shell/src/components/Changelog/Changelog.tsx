import * as React from 'react';
import { changelogEntries, ChangelogEntry } from './Changelog.entries';
import s from './Changelog.module.scss';

const ChangelogNode = React.memo(function ChangelogNode(props: ChangelogEntry) {
  return (
    <div className={s.entry}>
      <div className={s.title}>
        {props.title}
      </div>
      <div className={s.content}>
        {props.content}
      </div>
    </div>
  );
});

export const Changelog = React.memo(function Changelog() {
  const entriesNodes = changelogEntries.map((entry) => (
    <ChangelogNode
      key={entry.id}
      {...entry}
    />
  ));

  return (
    <div className={s.root}>
      {entriesNodes}
    </div>
  );
});
