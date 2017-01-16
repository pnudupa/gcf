function test_scrollAreaAndItemViewParts1() {
    var form = object("Application.ItemViewsForm")

    verify( form.checkTreeViewViewport( object("Application.ItemViewsForm/QTreeView[0]/viewport") ) )
    verify( form.checkTreeViewHorizontalScrollBar( object("Application.ItemViewsForm/QTreeView[0]/horizontalScrollBar") ) )
    verify( form.checkTreeViewVerticalScrollBar( object("Application.ItemViewsForm/QTreeView[0]/verticalScrollBar") ) )
    verify( form.checkTreeViewHorizontalHeader( object("Application.ItemViewsForm/QTreeView[0]/horizontalHeader") ) )

    verify( form.checkTableViewViewport( object("Application.ItemViewsForm/QTableView[0]/viewport") ) )
    verify( form.checkTableViewHorizontalScrollBar( object("Application.ItemViewsForm/QTableView[0]/horizontalScrollBar") ) )
    verify( form.checkTableViewVerticalScrollBar( object("Application.ItemViewsForm/QTableView[0]/verticalScrollBar") ) )
    verify( form.checkTableViewHorizontalHeader( object("Application.ItemViewsForm/QTableView[0]/horizontalHeader") ) )
    verify( form.checkTableViewVerticalHeader( object("Application.ItemViewsForm/QTableView[0]/verticalHeader") ) )

    verify( form.checkListViewViewport( object("Application.ItemViewsForm/QListView[0]/viewport") ) )
    verify( form.checkListViewVerticalScrollBar( object("Application.ItemViewsForm/QListView[0]/verticalScrollBar") ) )

    verify( form.checkScrollAreaViewport( object("Application.ItemViewsForm/QScrollArea[0]/viewport") ) )
    verify( form.checkScrollAreaHorizontalScrollBar( object("Application.ItemViewsForm/QScrollArea[0]/horizontalScrollBar") ) )
    verify( form.checkScrollAreaVerticalScrollBar( object("Application.ItemViewsForm/QScrollArea[0]/verticalScrollBar") ) )
}

function test_scrollAreaAndItemViewParts2() {
    var form = object("Application.ItemViewsForm")

    verify( form.checkTreeViewViewport( form.object("QTreeView[0]/viewport") ) )
    verify( form.checkTreeViewHorizontalScrollBar( form.object("QTreeView[0]/horizontalScrollBar") ) )
    verify( form.checkTreeViewVerticalScrollBar( form.object("QTreeView[0]/verticalScrollBar") ) )
    verify( form.checkTreeViewHorizontalHeader( form.object("QTreeView[0]/horizontalHeader") ) )

    verify( form.checkTableViewViewport( form.object("QTableView[0]/viewport") ) )
    verify( form.checkTableViewHorizontalScrollBar( form.object("QTableView[0]/horizontalScrollBar") ) )
    verify( form.checkTableViewVerticalScrollBar( form.object("QTableView[0]/verticalScrollBar") ) )
    verify( form.checkTableViewHorizontalHeader( form.object("QTableView[0]/horizontalHeader") ) )
    verify( form.checkTableViewVerticalHeader( form.object("QTableView[0]/verticalHeader") ) )

    verify( form.checkListViewViewport( form.object("QListView[0]/viewport") ) )
    verify( form.checkListViewVerticalScrollBar( form.object("QListView[0]/verticalScrollBar") ) )

    verify( form.checkScrollAreaViewport( form.object("QScrollArea[0]/viewport") ) )
    verify( form.checkScrollAreaHorizontalScrollBar( form.object("QScrollArea[0]/horizontalScrollBar") ) )
    verify( form.checkScrollAreaVerticalScrollBar( form.object("QScrollArea[0]/verticalScrollBar") ) )
}

