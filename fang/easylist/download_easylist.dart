import 'dart:io';
import 'package:http/http.dart' as http;

Future<void> main() async {
  final url = 'https://easylist.to/easylist/easylist.txt';
  final outputFile = 'easylist.txt';

  try {
    print('جاري تحميل الملف من: $url');
    
    final response = await http.get(Uri.parse(url));

    if (response.statusCode == 200) {
      final file = File(outputFile);
      await file.writeAsBytes(response.bodyBytes);
      print('تم تحميل الملف بنجاح: $outputFile');
      print('حجم الملف: ${response.bodyBytes.length} بايت');
    } else {
      print('خطأ: فشل التحميل. الكود: ${response.statusCode}');
    }
  } catch (e) {
    print('خطأ أثناء التحميل: $e');
  }
}
