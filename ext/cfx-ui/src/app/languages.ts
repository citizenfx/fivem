// #TODO: move to Intl API upon Chromium update
import cldrLanguages from 'cldr-data/main/en/languages.json';
import cldrSubTags from 'cldr-data/supplemental/likelySubtags.json';
import * as cldrjs from 'cldrjs';

import { L10nSchema } from "angular-l10n";
import languageRefs from 'webpack-extended-import-glob-loader!./languagerefs';

cldrjs.load(cldrLanguages, cldrSubTags);

const c = new cldrjs('en');

const languages = (languageRefs as any[]).map(lang => {
    const name = (lang.fileName as string).replace(/.*locale-(.*?)\.json/g, '$1').replace(/_/g, '-');
    const nameRef = name.split('-')[0];

    return {
        name,
        displayName: (c.main('localeDisplayNames/languages/' + nameRef) as string) + ` (${nameRef})`
    };
});

export class Languages {
    static toList(): L10nSchema[] {
        return languages.map(({name, displayName}) => ({
            locale: {
                language: name
            },
            dir: 'ltr',
            text: displayName
        }));
    }

    static toSettingsOptions() {
        const options: {[code: string]: string} = {};

        const languagesSet = languages;
        languagesSet.sort((a, b) => {
            return a.displayName.localeCompare(b.displayName);
        });

        for (const lang of languagesSet) {
            options[lang.name] = lang.displayName;
        }

        return options;
    }
}
