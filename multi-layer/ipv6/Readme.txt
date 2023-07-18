IPv6: 所有方法皆沒有16-segment table
Multilayer with B-tree :
	filename    :   multilayer-Btree.c
	compiler     :   gcc multilayer-Btree.c -o multilayer-Btree
	Execute	    :   ./multilayer-Btree ipv6_90build.txt ipv6_10insert.txt
	Note:裡頭的程式目前最終版本，含有len=48的hash table.
	若要測量10%miss的時間，Execute需改成如下:
	./multilayer-Btree ipv6rrc_all.txt ipv6_search_fail.txt
