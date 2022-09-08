import React from 'react';
import { observer } from "mobx-react-lite";
import { Indicator } from "cfx/ui/Indicator/Indicator";
import { Island } from "cfx/ui/Island/Island";
import { Text } from "cfx/ui/Text/Text";
import { $L } from "cfx/common/services/intl/l10n";
import { Pad } from "cfx/ui/Layout/Pad/Pad";
import { Page } from "cfx/ui/Layout/Page/Page";
import { Scrollable } from "cfx/ui/Layout/Scrollable/Scrollable";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { InsideNavBar } from "cfx/apps/mpMenu/parts/NavBar/InsideNavBar";
import { useService } from 'cfx/base/servicesContainer';
import { IChangelogService } from '../../services/changelog/changelog.service';
import { Prose } from 'cfx/ui/Prose/Prose';
import { Select, SelectOption } from 'cfx/ui/Select/Select';

export const ChangelogPage = observer(function ChangelogPage() {
  const ChangelogService = useService(IChangelogService);

  // Zeroing unseen versions counter
  React.useEffect(() => {
    ChangelogService.maybeMarkNewAsSeen();
  }, []);

  const versions = ChangelogService.versions;
  const versionItemsSelect: SelectOption<string>[] = React.useMemo(() => versions.map((version) => ({
    value: version,
    label: version,
  })), [versions]);

  return (
    <Page>
      <InsideNavBar>
        <Flex centered="axis">
          <Text size="large">
            {$L('#Changelogs')}
          </Text>

          <Select
            value={ChangelogService.selectedVersion}
            options={versionItemsSelect}
            onChange={ChangelogService.selectVersion}
          />
        </Flex>
      </InsideNavBar>

      <Island grow>
        <Scrollable>
          <Pad size='large'>
            <Prose>
              {ChangelogService.selectedVersionContent || <Indicator />}
            </Prose>
          </Pad>
        </Scrollable>
      </Island>
    </Page>
  );
});
