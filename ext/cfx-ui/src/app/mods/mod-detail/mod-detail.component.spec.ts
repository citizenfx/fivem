import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ModDetailComponent } from './mod-detail.component';

describe('ModDetailComponent', () => {
  let component: ModDetailComponent;
  let fixture: ComponentFixture<ModDetailComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ModDetailComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ModDetailComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
