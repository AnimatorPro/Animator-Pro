@echo off
del x.flc
del y.flc
@echo on
%1 circflic x.flc
%1 copyflic x.flc y.flc
%1 playfram x.flc
%1 custom   y.flc
%1 quickflc y.flc
