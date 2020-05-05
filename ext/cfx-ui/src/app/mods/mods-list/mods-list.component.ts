import { Component, OnInit, Input } from '@angular/core';
import { ModInfo } from '../mods.service';

@Component({
  selector: 'app-mods-list',
  templateUrl: './mods-list.component.html',
  styleUrls: ['./mods-list.component.scss']
})
export class ModsListComponent implements OnInit {
  @Input()
  source: ModInfo[];

  @Input()
  searchPlaceholder: string;

  searchText: string;

  constructor() { }

  ngOnInit(): void {
  }

  trackFn(mod: ModInfo) {
    return mod.modId;
  }

  updateFilters() {

  }
}
