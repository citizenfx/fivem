import { sendSdkMessage } from "../client/sendSdkMessage";
import { useKeyMapping } from "./utils";

export const SelectionController = new class SelectionController {
  private selectStart: number = 0;
  private selectedEntity: number | null = null;

  constructor() {
    on('we:clearSelection', () => {
      if (this.selectedEntity !== null) {
        SetEntityDrawOutline(this.selectedEntity, false);
        ReleaseScriptGuidFromEntity(this.selectedEntity);

        this.selectedEntity = null;
      }
    });
  }

  select(active: boolean) {
    if (active) {
      this.selectStart = GetGameTimer();
      return;
    }

    if (GetGameTimer() - this.selectStart > 250) {
      console.log('No select as >250ms hold');
      return;
    }

    const selectedEntity = SelectEntityAtCursor(6 | (1 << 5), true);
    if (selectedEntity) {
      if (this.selectedEntity !== null) {
        SetEntityDrawOutline(this.selectedEntity, false);
        ReleaseScriptGuidFromEntity(this.selectedEntity);

        this.selectedEntity = null;
      }

      if (this.selectedEntity !== selectedEntity) {
        this.selectedEntity = selectedEntity;
        SetEntityDrawOutline(this.selectedEntity, true);
      }
    }
  }

  getSelectedEntity(): number | null {
    return this.selectedEntity;
  }
};

const select = useKeyMapping('we_select', 'Select entity', 'mouse_button', 'mouse_left');
select.on(() => SelectionController.select(true));
select.off(() => SelectionController.select(false));
