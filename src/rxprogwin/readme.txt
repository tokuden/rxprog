                    SP6JTAG �\�t�g�E�F�A�E�ȈՃ}�j���A��
                       (C) Copyright 2009 ����d�q��H

���{�\�t�g�E�F�A�̊T�v
SP6JTAG�\�t�g�E�F�A�́ADOS�v�����v�g����Spartan-6�ɏ������݂��邽�߂̃\�t�g�E�F�A�ł��B

���Ή��f�o�C�X
XILINX: Spartan-6

���\�ȑ���
  �E��������
  �E����(�V���b�g�_�E��)

������
�{�\�t�g�E�F�A���g�p����O��NP1027 ���dSpartan-6�]���{�[�h�����p�ӂ��������B
sp6jtag.exe��system.bix��C�ӂ̃t�H���_�ɃR�s�[���Ă��������B

��������@
���R�}���h�̌n
SP6JTAG �́ADOS�v�����v�g����ȉ��̂悤�ɓ��͂��ċN�����܂��B

sp6jtag -command [bit�t�@�C����]

�������݂��s���ꍇ�ɂ́A�f�[�^�t�@�C�������w�肵�Ă��������B
XILINX FPGA�̏ꍇ�͊g���q��.bit�ł��B

���R�}���h�ꗗ
�R�}���h�ɂ͈ȉ��̂��̂�����܂�
  -detect  JTAG�`�F�[����̃f�o�C�X�������F�����܂�
  -bypass  ���̃f�o�C�X�ɑ΂��Ă͉������삵�܂���
  -auto    �����E�������݁E�x���t�@�C���s���܂�
  -write   �f�o�C�X�Ƀt�@�C�����������݂܂�
  -erase   �f�o�C�X���������܂�

���f�o�C�X�̎����F�����s��
jwriter -detect
�Ɠ��͂���ƁA�f�o�C�X�̎����F�����s���܂��B
JTAG�`�F�[���̓���`�F�b�N�Ȃǂɂ��g���������B

���������ݕ��@
JTAG�`�F�[���ɂP�̃f�o�C�X���ڑ�����Ă��āA���̃f�o�C�X�ɏ������ޏꍇ��
  jwriter -auto �f�[�^�t�@�C����
�ƋL�q���܂��B

���L�q��
�ESpartan-6����FPGA�ɁAmain.bit����������
sp6jtag -auto main.bit

�ESpartan-6����FPGA���F���ł��邩�ǂ�������
sp6jtag -detect
