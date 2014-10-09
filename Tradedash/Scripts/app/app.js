'use strict';

var baseUrl = window.location.origin = window.location.protocol + "//" + window.location.host + '/tradedash/api';

var phonecatApp = angular.module('tdDash', []);

phonecatApp.controller('TdCmparCtrl', function ($scope, $http) {

  $scope.Model = '';

  $scope.Init = function () {

    $http.get(baseUrl + '/gettradecompare/2014-09-01/2014-10-30').
      success(function (data, status, headers, config) {

        $scope.Model = data;
        console.info('gettradecompare success', data);
      }).
      error(function (data, status, headers, config) {

        console.info('gettradecompare failed', data);
      });
  };

});

var getErrorMsgFromResponse = function (response) {

  var ret = '';

  if (response.ExceptionMessage) ret += response.ExceptionMessage;

  return ret;
}