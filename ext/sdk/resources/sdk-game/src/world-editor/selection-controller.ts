import { useKeyMapping } from "./utils";

export const SelectionController = new class SelectionController {
  private selectStart: number = 0;
  private selectedEntity: number | null = null;

  private disabled = false;

  enable() {
    this.disabled = false;
  }

  disable() {
    this.selectStart = 0;
    this.disabled = true;
  }

  select(active: boolean) {
    if (this.disabled) {
      return;
    }

    if (active) {
      this.selectStart = GetGameTimer();
      return;
    }

    if (GetGameTimer() - this.selectStart > 250) {
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

  setSelectedEntity(entity: number | null) {
    if (this.selectedEntity !== null) {
      SetEntityDrawOutline(this.selectedEntity, false);
      ReleaseScriptGuidFromEntity(this.selectedEntity);

      this.selectedEntity = null;
    }

    this.selectedEntity = entity;

    if (this.selectedEntity !== null) {
      SetEntityDrawOutline(this.selectedEntity, true);
    }
  }
};

const select = useKeyMapping('we_select', 'Select entity', 'mouse_button', 'mouse_left');
select.on(() => SelectionController.select(true));
select.off(() => SelectionController.select(false));
