object FormWait: TFormWait
  Left = 437
  Top = 424
  BorderStyle = bsSingle
  Caption = 'Waiting...'
  ClientHeight = 181
  ClientWidth = 315
  Color = clBtnFace
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = #65325#65331' '#65328#12468#12471#12483#12463
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 12
  object Shape1: TShape
    Left = 8
    Top = 112
    Width = 297
    Height = 25
  end
  object Button1: TButton
    Left = 128
    Top = 144
    Width = 75
    Height = 25
    Cancel = True
    Caption = #12461#12515#12531#12475#12523
    TabOrder = 0
    OnClick = Button1Click
  end
  object Memo1: TMemo
    Left = 8
    Top = 8
    Width = 297
    Height = 97
    ReadOnly = True
    TabOrder = 1
  end
  object Timer1: TTimer
    Interval = 100
    OnTimer = Timer1Timer
    Left = 272
    Top = 144
  end
end
