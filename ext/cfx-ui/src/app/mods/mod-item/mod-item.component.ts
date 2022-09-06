import { Component, OnInit, Input } from '@angular/core';
import { ModInfo } from '../mods.service';

@Component({
  selector: 'app-mod-item',
  templateUrl: './mod-item.component.html',
  styleUrls: ['./mod-item.component.scss']
})
export class ModItemComponent implements OnInit {
  @Input()
  mod: ModInfo;

  constructor() { }

  ngOnInit(): void {
  }

}
