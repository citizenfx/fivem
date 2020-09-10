import { interfaces, injectable } from 'inversify';
import { workspacePreferenceSchema } from '@theia/workspace/lib/browser/workspace-preferences';
import {
  PreferenceSchema,
  PreferenceSchemaProvider,
  PreferenceProviderDataChange
} from '@theia/core/lib/browser/preferences';

@injectable()
class FxdkPreferenceSchemaProvider extends PreferenceSchemaProvider {
  protected doSetSchema(schema: PreferenceSchema): PreferenceProviderDataChange[] {
    // Disabling workspace preferences
    if (schema === workspacePreferenceSchema) {
      return [];
    }

    return super.doSetSchema(schema);
  }
}

export function rebindWorkspacePreferences(rebind: interfaces.Rebind): void {
  rebind(PreferenceSchemaProvider).to(FxdkPreferenceSchemaProvider).inSingletonScope();
}
