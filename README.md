# mod_dynamic_include.c

## description

 * クライアントが接続した時に動的に設定ファイルを読み込みProftpdモジュール
 * アカウント別に大量の設定ファイルを管理する必要がある環境で便利だと思います
   * ex. 共有ホスティングサーバーなど

## usage

 * /etc/proftpd.conf
    
```
LoadModule mod_dynamic_include.c
DynamicIncludePath /etc/proftpd/%u.conf
```

## format spec

```
%u               user name ( sent by 'USER' command )
```
