import { ScrollContainer } from 'components/ScrollContainer/ScrollContainer';
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
  const entriesNodes = changelogEntries.map((entry, index) => {
    if (index === 0) {
      return (
        <>
          <ChangelogNode
            key={entry.id}
            {...entry}
          />

          <div className={s.older}>Older entries</div>
        </>
      );
    }

    return (
      <ChangelogNode
        key={entry.id}
        {...entry}
      />
    );
  });

  return (
    <ScrollContainer className={s.root}>
      {entriesNodes}
    </ScrollContainer>
  );
});
